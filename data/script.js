
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
  initWebSocket();
}

function getValues() {
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
  console.log(sliderValue);
  websocket.send("Brightness=" + sliderValue.toString());
}

function processCheckbox(element) {
  var checkboxState = document.getElementById(element.id).checked;
  var boolState = 0;
  if (checkboxState === true) {
    boolState = 1;
  }
  return boolState;
}

function toggleDialect(element) {
  var boolState = processCheckbox(element);
  websocket.send("Dialect=" + boolState);
}

function toggleThreeQuater(element) {
  var boolState = processCheckbox(element);
  websocket.send("ThreeQuater=" + boolState);
}

function toggleQuaterPast(element) {
  var boolState = processCheckbox(element);
  websocket.send("QuaterPast=" + boolState);
}

function updateMainColor(element) {
  var colorValue = document.getElementById(element.id).value;
  console.log(colorValue.replace("#", ""));
  websocket.send("MainColor=" + colorValue.replace("#", ""));
}

function colorToHex(color) {
  var hexadecimal = color.toString(16);
  return hexadecimal.length == 1 ? "0" + hexadecimal : hexadecimal;
}

function convertRGBtoHex(red, green, blue) {
  return "#" + colorToHex(red) + colorToHex(green) + colorToHex(blue);
}

function onMessage(event) {
  console.log(event.data);
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
    } else {
      document.getElementById(key).value = myObj[key];
    }
  }
}
