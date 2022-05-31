
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

function toggleDialect(element) {
  var checkboxState = document.getElementById(element.id).checked;
  console.log(checkboxState);
  var boolState = "false";
  if (checkboxState === true) {
    boolState = "true";
  }
  websocket.send("Dialect=" + boolState);
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
      if (myObj[key] === "true") {
        elem.checked = true;
      } else {
        elem.checked = false;
      }
    } else {
      document.getElementById(key).value = myObj[key];
    }
  }
}
