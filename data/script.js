
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var maxBits = 32;
var allowSavingWordConfig = true;
var nextWordIndex = 0;
window.addEventListener('load', onload);

setInterval(function () {
  websocket.send("getTime");
}, 60000)

function onload(event) {
  initWebSocket();
}

function getValues() {
  websocket.send("getTime");
  websocket.send("getValues");
}

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log('Connection opened');
  allowSavingWordConfig = true;
  nextWordIndex = 0;
  getValues();
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

function updateSliderBrightness(element) {
  var sliderValue = document.getElementById(element.id).value / 100.0;
  console.log(sliderValue);
  document.getElementById(element.id + "Display").innerHTML = Math.round(sliderValue * 100.0);
  websocket.send(element.id + "=" + sliderValue.toString());
}

function getTime() {
  websocket.send("getTime")
}

function processCheckbox(element, id) {
  var checkboxState = document.getElementById(element.id).checked;
  var boolState = 0;
  if (checkboxState === true) {
    boolState = 1;
  }
  websocket.send(element.id + "=" + boolState);
}

function hexToColor(hex) {
  const r = parseInt(hex.slice(1, 3), 16);
  const g = parseInt(hex.slice(3, 5), 16);
  const b = parseInt(hex.slice(5, 7), 16);
  return [r, g, b];
}

function processColorPicker(element) {
  var colorValue = document.getElementById(element.id).value;
  var rgb = hexToColor(colorValue);
  websocket.send(element.id + "=" + JSON.stringify(rgb));
}

function processTextInput(element, event) {
  if (event.key === "Enter") {
    var offset = document.getElementById(element.id);
    websocket.send(element.id + "=" + offset.value);
  }
}

function wordConfigFillRow(message) {
  var table = document.getElementById("word-config");
  var index = message["index"];

  if (index + 2 >= table.rows.length) {
    addRow("word-config");
  }
  var row = table.rows[index + 2]
  if (row === undefined) {
    return;
  }

  for (let cell of row.cells) {
    let content = cell.childNodes[0];

    if (content === undefined)
      continue

    switch (content.id) {
      case "leds":
        var ledString = "";
        index = 0;
        for (let j of message["leds"]) {
          for (var k = 0; k < maxBits; ++k) {
            if ((((1 << k) & j) >>> 0) > 0) {
              if (ledString !== "") {
                ledString += ",";
              }
              ledString += ((k + maxBits * index) + 1)
            }
          }
          ++index;
        }
        content.value = ledString;
        break;
      case "color":
        content.value = "#" + message["color"].toString(16).padStart(6,'0');
        break;
      case "enable":
        content.checked = message["enable"];
        break;
      case "useTime":
        content.checked = message["useTime"];
        break;
      case "when":
        content.selectedIndex = message["when"];
        break;
      case "date":
        if (message["date"]["day"] == 0 || message["date"]["month"] == 0) {
          content.value = "";
        } else {
          content.value = message["date"]["day"] + "." + message["date"]["month"];
        }
        break;
      default:
        break;
    }
  }
}

function wordConfigRemoveRows(numRows) {
  var table = document.getElementById("word-config");
  if (numRows + 2 <= table.rows.length) {
    return;
  }
  var rowsToRemove = table.rows.length - numRows + 2;
  for (var i = 0; i < rowsToRemove; ++i) {
    let row = table.rows[table.rows.length - 1];
    if (row === undefined) {
      continue
    }
    row.remove();
  }
}

function onMessage(event) {
  if (event.data === "null") {
    console.log("Received data is null");
    return
  }
  try {
    var message = JSON.parse(event.data);
  } catch (e) {
    console.log(event.data)
    console.error(e);
  }
  var keys = Object.keys(message);

  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (document.getElementById(key + "Display") != null) {
      document.getElementById(key + "Display").innerHTML = Math.round(message[key] * 100.0);
    }
    elem = document.getElementById(key);
    if (elem != null) {
      if (key == "brightness" || key == "backgroundBrightness") {
        document.getElementById(key).value = Math.round(message[key] * 100.0);
      } else if (elem.getAttribute("type") === "checkbox") {
        elem.checked = message[key];
      } else if (elem.getAttribute("type") === "color") {
        document.getElementById(key).value = "#" + message[key].toString(16).padStart(6, '0');
      } else {
        document.getElementById(key).value = message[key];
      }
    }

    if (key === "wordTime" || key === "uptime") {
      document.getElementById(key).innerHTML = message[key];
    }

    if (key === "wordConfig") {
      wordConfigFillRow(message[key])
      let index = message[key]["index"] + 1;
      websocket.send("continueSendWordConfig=" + index);
    }

    if (key == "wordConfigNumRows") {
      wordConfigRemoveRows(message[key]);
    }

    if (key === "activeLeds") {
      var table = document.getElementById("preview");

      var j = 0;
      var k = 0;
      for (let row of table.rows) {
        for (let cell of row.cells) {
          if (j < parseInt(message[key]["startIdx"])) {
            ++j;
            continue;
          }
          if (k >= parseInt(message[key]["len"])) {
            break;
          }
          cell.style.color = "#" + message[key]["leds"][k++];
        }
        if (parseInt(k >= message[key]["len"])) {
          break;
        }
      }
      var endIdx = k + parseInt(message[key]["startIdx"]);
      websocket.send("continueSendPreview=" + endIdx);
    }

    if (key === "continueWordConfig" && nextWordIndex > 0) {
      allowSavingWordConfig = true;
      saveWords(nextWordIndex);
    }
  }
}

function createTable(tableData) {
  var tableBody = document.getElementById('previewBody');
  var counter = 1;

  tableData.forEach(function (rowData) {
    var row = document.createElement('tr');
    row.classList.add("tr_preview")

    rowData.forEach(function (cellData) {
      var cell = document.createElement('td');
      cell.classList.add("td_preview")
      cell.setAttribute("data-tag", counter++)
      cell.appendChild(document.createTextNode(cellData));
      row.appendChild(cell);
    });

    tableBody.appendChild(row);
  });
}

function addRow(tableID) {
  var table = document.getElementById(tableID);
  var rowCount = table.rows.length;
  var row = table.insertRow(rowCount);

  row.classList.add("word_tr")
  var colCount = table.rows[1].cells.length;

  for (var i = 0; i < colCount; i++) {

    var newcell = row.insertCell(i);
    newcell.classList.add("word_td")

    switch (i) {
      case 0:
        newcell.innerHTML = '<td class="word_td"><input type="checkbox", id="enable"></td>';
        break;
      case 1:
        newcell.innerHTML = '<td class="word_td"><input type="checkbox", id="useTime"></td>';
        break;
      case 2:
        newcell.innerHTML = '<td class="word_td"><input type="text", id="leds"></td>';
        break;
      case 3:
        newcell.innerHTML = '<td class="word_td"><input type="color", id="color"></td>';
        break;
      case 4:
        newcell.innerHTML = '<td class="word_td"><select name="When", id="when"><option value="always">Always</option><option value="date">Date</option></select></td>';
        break;
      case 5:
        newcell.innerHTML = '<td class="word_td"><input type="text", id="date"></td>';
        break;
      case 6:
        newcell.innerHTML = '<td class="word_td"><button type="button">Delete</button></td>';
        break;
      default:
        newcell.innerHTML = "Unknown";
        break;
    }

    switch (newcell.childNodes[0].type) {
      case "text":
        newcell.childNodes[0].value = "";
        break;
      case "color":
        newcell.childNodes[0].value = "#FCB821";
        break;
      case "checkbox":
        newcell.childNodes[0].checked = true;
        break;
      case "select-one":
        newcell.childNodes[0].selectedIndex = 1;
        break;
      case "button":
        newcell.childNodes[0].addEventListener("click", function (event) {
          $(this).closest('tr').remove();
        });
        break;
    }
  }
}

function validateDate(date) {
  var dateSplit = date.split(".");
  if (dateSplit.length != 2 || isNaN(dateSplit[0]) || isNaN(dateSplit[1])) {
    return false;
  }
  var day = parseInt(dateSplit[0]);
  var month = parseInt(dateSplit[1]);
  if ((month == 2 && day > 29)
    || month > 12
    || month < 1
    || day < 1
    || (month % 2 == 0 && month < 8 && day > 30)
    || (month % 2 == 1 && month < 8 && day > 31)
    || (month % 2 == 0 && month > 7 && day > 31)
    || (month % 2 == 1 && month > 7 && day > 30)
  ) {
    return false;
  }
  return true
}

function hideError(hideError, errorMsg, elementId) {
  var elem = document.getElementById(elementId)
  if (hideError) {
    elem.style.display = "none";
  } else {
    elem.textContent = errorMsg;
    elem.style.display = "block";
  }
}

function saveWords(rowIndex) {
  var table = document.getElementById("word-config");
  var useDate = true;

  if (allowSavingWordConfig === false) {
    console.log("Not allowed to save words")
    return;
  }

  if (rowIndex == 2 && table.rows.length == 2) {
    websocket.send("clearWordConfig");
  }

  if (rowIndex >= table.rows.length) {
    allowSavingWordConfig = true;
    nextWordIndex = 0;
    websocket.send("finishedWordConfig");
    return;
  }

  var validDate = true;
  var invalidDate = "";
  var validLeds = true;
  var invalidLeds = "";

  row = table.rows[rowIndex];

  var jsonContent = {};
  for (let cell of row.cells) {
    let content = cell.childNodes[0];
    var ledHex = [];

    if (content === undefined)
      continue

    switch (content.id) {
      case "leds":
        leds = content.value.split(",");
        if (leds.length == 1 && leds[0] === "") {
          validLeds = false;
          break;
        }
        for (let j of leds) {
          if (j === "" || isNaN(j) || parseInt(j) < 1) {
            validLeds = false;
            invalidLeds = content.value;
            break;
          }
        }
        for (let j of leds) {
          var led = parseInt(j) - 1;
          var ledIndex = Math.floor(led/maxBits);
          if (isNaN(ledHex[ledIndex])) {
            ledHex[ledIndex] = 0;
          }
          ledHex[ledIndex] += Math.pow(2, led - ledIndex * maxBits)
        }
        var k = 0;
        for (let j of ledHex) {
          if (j === undefined) {
            ledHex[k] = 0;
          }
          ++k;
        }
        jsonContent["leds"] = ledHex
        break;
      case "color":
        jsonContent["color"] = hexToColor(content.value);
        break;
      case "enable":
        jsonContent["enable"] = content.checked;
        break;
      case "useTime":
        jsonContent["useTime"] = content.checked;
        break;
      case "when":
        jsonContent["when"] = content.selectedIndex;
        if (content.options[content.selectedIndex].text == "Always") {
          useDate = false
        }
        break;
      case "date":
        if (!validateDate(content.value) && (useDate || (!useDate && content.value != ""))) {
          validDate = false;
          invalidDate = content.value;
          break;
        }
        var dateSplit = content.value.split(".");
        jsonContent["date"] = { "day": parseInt(dateSplit[0]), "month": parseInt(dateSplit[1]) };
        break;
      default:
        break;
    }
  }
  hideError(validDate, "Error in date: " + invalidDate, "dateError");
  hideError(validLeds, "Error in LEDs: " + invalidLeds, "ledError");

  if (validDate && validLeds) {
    if (rowIndex === 2) {
      websocket.send("clearWordConfig");
    }
    websocket.send("wordConfig=" + JSON.stringify(jsonContent));
    allowSavingWordConfig = false;
    nextWordIndex = rowIndex + 1
  }
}

function setTitles(title1, title2) {
  document.getElementById("title1").innerHTML = title1
  document.getElementById("title2").innerHTML = title2
}

createTable(letters["clock"]);

setTitles(title1, title2);


