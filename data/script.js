
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
  console.log([r, g, b]);
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

function colorToHex(color) {
  var hexadecimal = color.toString(16);
  return hexadecimal.length == 1 ? "0" + hexadecimal : hexadecimal;
}

function convertRGBtoHex(red, green, blue) {
  return "#" + colorToHex(red) + colorToHex(green) + colorToHex(blue);
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

    if (key == "timeColor") {
      var color = myObj[key];
      timeColor = convertRGBtoHex(color[0], color[1], color[2]);
    }

    if (key === "wordTime") {
      document.getElementById(key).innerHTML = myObj[key];
    }

    if (key === "wordConfig") {
      console.log("Restore word config");
    }

    if (key === "activeLeds") {
      var table = document.getElementById("preview");
      var backgroundColor = document.getElementById("backgroundColor").value;
      var useBackgroundColor = document.getElementById("switchBackgroundColor").checked;

      if (!useBackgroundColor)
        backgroundColor = "white";

      var i = 0;
      for (let row of table.rows) {
        var leds = myObj[key][i];
        var j = 0;
        for (let cell of row.cells) {
          if (leds & (1 << j))
            cell.style.backgroundColor = timeColor;
          else
            cell.style.backgroundColor = backgroundColor;
          ++j;
        }
        ++i;
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
        newcell.innerHTML = '<td class="word_td"><input type="checkbox""></td>';
        break;
      case 1:
        newcell.innerHTML = '<td class="word_td"><input type="text""></td>';
        break;
      case 2:
        newcell.innerHTML = '<td class="word_td"><input type="color""></td>';
        break;
      case 3:
        newcell.innerHTML = '<td class="word_td"><select name="When"><option value="always">Always</option><option value="date">Date</option></select></td>';
        break;
      case 4:
        newcell.innerHTML = '<td class="word_td"><input type="date""></td>';
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
        newcell.childNodes[0].value = "GLÜCK";
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
        newcell.childNodes[0].addEventListener("click", function(event){
          $(this).closest('tr').remove();
        });
        break;
    }
  }
}

function saveWords() {
  var table = document.getElementById("word-config");
  var data = new Array();
  var i = 0;
  for (let row of table.rows) {
    if (i === 0) {
      ++i;
      continue
    }
    let jsonContent = {};
    for (let cell of row.cells) {
      let content = cell.childNodes[0];

      if (content === undefined)
        continue

      switch (content.type) {
        case "text":
          jsonContent["words"] = content.value;
          break;
        case "color":
          jsonContent["color"] = hexToColor(content.value);
          break;
        case "checkbox":
          jsonContent["enable"] = content.checked;
          break;
        case "select-one":
          jsonContent["when"] = content.options[content.selectedIndex].text;
          break;
        case "date":
          var dateVal = new Date(content.value);
          jsonContent["date"] = {};
          jsonContent["date"]["valid"] = false;
          if (dateVal instanceof Date && !isNaN(dateVal)) {
            jsonContent["date"] = {"day": dateVal.getDate(), "month": dateVal.getMonth() + 1};
            jsonContent["date"]["valid"] = true;
          }
          break;
        default:
          if (i === 1)
            jsonContent["words"] = "TIME";

      }
    }
    data.push(jsonContent);
    ++i;
  }
  console.log("Save word config");
  console.log(data);
  websocket.send("wordConfig=" + JSON.stringify(data));
}

createTable(letters["clock"])
