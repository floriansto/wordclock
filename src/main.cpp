#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <ESPAsyncTCP.h>
#include <NTPClient.h>
#include <RTClib.h>

#include <Arduino_JSON.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <math.h>
#include <timeprocessor.h>

#include "../include/hw_settings.h"
#include "../include/main.h"
#include "../include/settings.h"
#include "../include/timeUtils.h"
#include "LittleFS.h"

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
    ROW_PIXELS, COL_PIXELS, PIN,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncWebSocket ws("/ws");

WiFiUDP ntpUDP;
RTC rtc;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

Settings *settings = new Settings();
TimeProcessor *timeProcessor = new TimeProcessor(
    settings->getUseDialect(), settings->getUseQuaterPast(),
    settings->getUseThreeQuater(), offsetLowSecs, offsetHighSecs, NUMPIXELS);

Error error = Error::OK;
bool wifiConnected = false;
u_int16_t active_leds_loop = 0;

double calcBrightnessScale(u_int16_t active_leds) {
  if (active_leds == 0) {
    active_leds = NUMPIXELS;
  }
  double current_per_color = max_current_ma / (double)active_leds / 3.0;
  return current_per_color / max_current_per_color_ma;
}

void notifyClients(String settingsValues) { ws.textAll(settingsValues); }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char *)data;
    if (message.indexOf("Brightness") == 0) {
      int brightness = message.substring(message.indexOf("=") + 1).toInt();
      settings->setBrightness(brightness);
      Serial.println(brightness);
      Serial.print(settings->getJsonString());
      notifyClients(settings->getJsonString());
    }
    if (message.indexOf("Dialect") == 0) {
      String strDialect = message.substring(message.indexOf("=") + 1);
      settings->setUseDialect(strDialect == "true");
      timeProcessor->setDialect(settings->getUseDialect());
      Serial.println(strDialect);
      Serial.print(settings->getJsonString());
      notifyClients(settings->getJsonString());
    }

    if (strcmp((char *)data, "getValues") == 0) {
      notifyClients(settings->getJsonString());
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                  client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initWebFunctions() {
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");

  server.begin();
  Serial.println("HTTP server started");

  initWebSocket();

  timeClient.begin();
  Serial.println("NTP client started");
  timeClient.update();
}

void stopWebFunctions() {
  server.end();
  Serial.println("HTTP server stopped");

  timeClient.end();
  Serial.println("NTP client stopped");
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(settings->getBrightness() *
                       calcBrightnessScale(NUMPIXELS));
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  // wifiManager.setConfigPortalBlocking(false);

  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
    wifiConnected = true;
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
  }

  if (!rtc.rtc.begin()) {
    Serial.println("Couldn't find RTC");
    rtc.found = false;
  }

  if (rtc.found == true && rtc.rtc.lostPower()) {
    rtc.valid = false;
  }

  initFS();

  if (wifiConnected == true) {
    initWebFunctions();
    if (adjustSummertime(&rtc, &timeClient, settings->getUtcHourOffset(),
                         wifiConnected) != true) {
      error = Error::SUMMERTIME_ERROR;
    }
  }
}

unsigned long lastRun = 0;
u_int16_t evalTimeEvery = 1000;
unsigned long lastDaylightCheck = 0;
u_int32_t checkDaylightTime = 1000;
unsigned long lastRtcSync = 0;
u_int32_t syncRtc = 24 * 3600;
unsigned long lastTimeUpdate = 0;
u_int32_t updateTime = 1 * 1000;

void loop() {
  u_int16_t color;
  error = Error::OK;

  // wifiManager.process();
  if (!wifiConnected && WiFi.status() == WL_CONNECTED) {
    initWebFunctions();
  }
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    stopWebFunctions();
  }
  wifiConnected = WiFi.status() == WL_CONNECTED;

  TIME time = getTime(&rtc, &timeClient, wifiConnected);
  if (!time.valid) {
    error = Error::NO_TIME;
  }

  if (time.valid &&
      !timeProcessor->update(time.hour, time.minute, time.seconds)) {
    Serial.println("Error occured");
    error = Error::TIME_TO_WORD_CONVERSION;
  }

  if (wifiConnected == true &&
      millis() - lastDaylightCheck > checkDaylightTime) {
    if (adjustSummertime(&rtc, &timeClient, settings->getUtcHourOffset(),
                         wifiConnected) != true) {
      error = Error::SUMMERTIME_ERROR;
    }
    lastDaylightCheck = millis();
  }

  if (millis() - lastRun > evalTimeEvery) {
    lastRun = millis();
    Serial.println("===========");
    TIME ntpTime = getTimeNtp(&timeClient);
    Serial.print(ntpTime.hour);
    Serial.print(":");
    Serial.print(ntpTime.minute);
    Serial.print(":");
    Serial.println(ntpTime.seconds);
    TIME rtcTime = getTimeRtc(&(rtc.rtc));
    Serial.print(rtcTime.hour);
    Serial.print(":");
    Serial.print(rtcTime.minute);
    Serial.print(":");
    Serial.println(rtcTime.seconds);
  }

  if (millis() - lastRtcSync > syncRtc && rtc.found == true) {
    if (updateRtcTime(&rtc, &time, wifiConnected) == false) {
      error = Error::UPDATE_RTC_TIME_ERROR;
    }
    lastRtcSync = millis();
  }

  color = matrix.Color(0, 255, 0);
  if (error != Error::OK) {
    color = matrix.Color(255, 0, 0);
  }

  if (millis() - lastTimeUpdate > updateTime && error == Error::OK) {
    Timestack *stack = timeProcessor->getStack();
    TIMESTACK elem;
    int buffer[4];

    matrix.fillScreen(0);
    active_leds_loop = 0;
    for (int i = 0; i < stack->getSize(); ++i) {
      if (!stack->get(&elem, i)) {
        error = Error::TIMESTACK_GET_ELEM_FAILED;
        return;
      }
      get_led_rectangle(elem.state, elem.useDialect, buffer);
      matrix.fillRect(buffer[0], buffer[1], buffer[2], buffer[3], color);
      active_leds_loop += (buffer[2] * buffer[3]);
    }
    lastTimeUpdate = millis();
  }

  matrix.setBrightness(settings->getBrightness() *
                       calcBrightnessScale(active_leds_loop));
  matrix.show();

  delay(10);
}
