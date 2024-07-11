
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
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
  getValues();
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

function updateSliderBrightness(element) {
  var sliderValue = document.getElementById(element.id).value;
  document.getElementById(element.id + "Display").innerHTML = sliderValue;
  websocket.send("Brightness=" + sliderValue.toString());
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

function intToRgbColor(intValue) {
  // Extract the red, green, and blue components from the integer
  var red = (intValue >> 16) & 0xFF;
  var green = (intValue >> 8) & 0xFF;
  var blue = intValue & 0xFF;

  // Return the RGB color values as an array
  return [red, green, blue];
}

function processColorPicker(element) {
  var colorValue = document.getElementById(element.id).value;
  console.log(colorValue)
  var rgb = hexToColor(colorValue);
  websocket.send(element.id + "=" + JSON.stringify(rgb));
}

function processTextInput(element, event) {
  if (event.key === "Enter") {
    var offset = document.getElementById(element.id);
    websocket.send(element.id + "=" + offset.value);
  }
}

function colorToHex(color) {
  var hexadecimal = color.toString(16);
  return hexadecimal.length == 1 ? "0" + hexadecimal : hexadecimal;
}

function convertRGBtoHex(red, green, blue) {
  return "#" + colorToHex(red) + colorToHex(green) + colorToHex(blue);
}

function setTable(myObj, key) {
  var table = document.getElementById("word-config");
  var counter = 0;
  var initialTableRows = table.rows.length - 2
  var rowsToRemove = 0;

  if (initialTableRows > myObj[key].length) {
    var rowsToRemove = initialTableRows - myObj[key].length;
  }

  console.log("Table rows: " + initialTableRows)
  console.log("Rows to remove: " + rowsToRemove);
  for (var i = 0; i < rowsToRemove; ++i) {
    let row = table.rows[table.rows.length - 1];
    if (row === undefined) {
      continue
    }
    console.log("Remove row: " + i);
    console.log("Table rows: " + table.rows.length)
    console.log(row)
    row.remove();
  }

  counter = 0
  for (let i of myObj[key]) {
    if (counter >= initialTableRows) {
      addRow("word-config");
      console.log("Add Row");
    }

    let row = table.rows[counter + 2]
    if (row === undefined) {
      console.log("Row " + counter + 2 + " does not exist");
      continue;
    }

    for (let cell of row.cells) {
      let content = cell.childNodes[0];

      if (content === undefined)
        continue

      switch (content.id) {
        case "leds":
          content.value = i["leds"].toString();
          break;
        case "color":
          content.value = convertRGBtoHex(i["color"][0], i["color"][1], i["color"][2]);
          break;
        case "enable":
          content.checked = i["enable"];
          break;
        case "when":
          for (var j = 0; j < content.options.length; ++j) {
            if (content.options[j].text === i["when"]) {
              content.selectedIndex = j;
              break;
            }
          }
          break;
        case "date":
          if (i["date"]["day"] == null || i["date"]["month"] == null) {
            content.value = "";
          } else {
            content.value = i["date"]["day"] + "." + i["date"]["month"];
          }
          break;
        default:
          break;
      }
    }
    ++counter;
  }
  return;
}

function onMessage(event) {
  if (event.data === "null") {
    console.log("Received data is null");
    return
  }
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);
  var timeColor = "#FF0000";

  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (document.getElementById(key + "Display") != null) {
      document.getElementById(key + "Display").innerHTML = myObj[key];
    }
    elem = document.getElementById(key);
    if (elem != null) {
      if (elem.getAttribute("type") === "checkbox") {
        elem.checked = myObj[key];
      } else if (elem.getAttribute("type") === "color") {
        var color = myObj[key];
        document.getElementById(key).value = convertRGBtoHex(color[0], color[1], color[2]);
      } else {
        document.getElementById(key).value = myObj[key];
      }
    }

    console.log("Got key " + key);

    if (key == "timeColor") {
      var color = myObj[key];
      timeColor = convertRGBtoHex(color[0], color[1], color[2]);
    }

    if (key === "wordTime") {
      document.getElementById(key).innerHTML = myObj[key];
    }

    if (key === "wordConfig") {
      setTable(myObj, key);
    }

    if (key === "activeLeds") {
      var table = document.getElementById("preview");

      var j = 0;
      for (let row of table.rows) {
        for (let cell of row.cells) {
          intColor = parseInt(myObj[key][j++]);
          if (intColor == 0)
            intColor = 0xFFFFFF;
          rgb = intToRgbColor(intColor);
          cell.style.color = convertRGBtoHex(rgb[0], rgb[1], rgb[2]);
        }
      }
    }
  }
}

function createTable(tableData) {
  var table = document.getElementById('preview');
  var tableBody = document.createElement('tbody');

  tableData.forEach(function (rowData) {
    var row = document.createElement('tr');
    row.classList.add("tr_preview")

    rowData.forEach(function (cellData) {
      var cell = document.createElement('td');
      cell.classList.add("td_preview")
      cell.appendChild(document.createTextNode(cellData));
      row.appendChild(cell);
    });

    tableBody.appendChild(row);
  });

  table.appendChild(tableBody);
  document.body.appendChild(table);
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
        newcell.innerHTML = '<td class="word_td"><input type="text", id="leds"></td>';
        break;
      case 2:
        newcell.innerHTML = '<td class="word_td"><input type="color", id="color"></td>';
        break;
      case 3:
        newcell.innerHTML = '<td class="word_td"><select name="When", id="when"><option value="always">Always</option><option value="date">Date</option></select></td>';
        break;
      case 4:
        newcell.innerHTML = '<td class="word_td"><input type="text", id="date"></td>';
        break;
      case 5:
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

function saveWords() {
  var table = document.getElementById("word-config");
  var data = new Array();
  var i = 0;
  var validDate = true;
  var invalidDate = "";
  var validLeds = true;
  var invalidLeds = "";
  var useDate = true;
  for (let row of table.rows) {
    if (i < 2) {
      ++i;
      continue
    }
    let jsonContent = {};
    for (let cell of row.cells) {
      let content = cell.childNodes[0];

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
          jsonContent["leds"] = leds.map(element => {
            return parseInt(element);
          });
          break;
        case "color":
          jsonContent["color"] = hexToColor(content.value);
          break;
        case "enable":
          jsonContent["enable"] = content.checked;
          break;
        case "when":
          jsonContent["when"] = content.options[content.selectedIndex].text;
          if (jsonContent["when"] == "Always") {
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
    data.push(jsonContent);
    ++i;
  }

  hideError(validDate, "Error in date: " + invalidDate, "dateError");
  hideError(validLeds, "Error in LEDs: " + invalidLeds, "ledError");
  console.log(data)

  if (validDate && validLeds) {
    websocket.send("wordConfig=" + JSON.stringify(data));
  }
}

function setTitles(title1, title2) {
  document.getElementById("title1").innerHTML = title1
  document.getElementById("title2").innerHTML = title2
}

createTable(letters["clock"]);

setTitles(title1, title2);


