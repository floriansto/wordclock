
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

function processColorPicker(element) {
  var colorValue = document.getElementById(element.id).value;
  websocket.send(element.id + "=" + colorValue.replace("#", ""));
}

function colorToHex(color) {
  var hexadecimal = color.toString(16);
  return hexadecimal.length == 1 ? "0" + hexadecimal : hexadecimal;
}

function convertRGBtoHex(red, green, blue) {
  return "#" + colorToHex(red) + colorToHex(green) + colorToHex(blue);
}

function onMessage(event) {
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);

  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (document.getElementById(key + "Display") != null) {
      document.getElementById(key + "Display").innerHTML = myObj[key];
    }
    elem = document.getElementById(key);
    if (elem.getAttribute("type") === "checkbox") {
      if (myObj[key] === 1) {
        elem.checked = true;
      } else {
        elem.checked = false;
      }
    } else if (elem.getAttribute("type") === "color") {
      var color = myObj[key];
      document.getElementById(key).value = convertRGBtoHex(color.r, color.g, color.b);
    } else if (key === "wordTime") {
      document.getElementById(key).innerHTML = myObj[key];
    } else {
      document.getElementById(key).value = myObj[key];
    }
  }
}

function createTable(tableData) {
  var table = document.getElementById('preview');
  var tableBody = document.createElement('tbody');

  console.log(tableData["clock"])

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

createTable(letters["clock"])
