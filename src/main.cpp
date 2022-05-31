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
RTC_DS3231 rtc;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

Settings *settings = new Settings();
TimeProcessor *timeProcessor = new TimeProcessor(
    settings->getUseDialect(), settings->getUseQuaterPast(),
    settings->getUseThreeQuater(), offsetLowSecs, offsetHighSecs, NUMPIXELS);

bool rtcFound = true;
bool rtcValid = true;
bool error = false;
bool wifiConnected = false;
u_int16_t active_leds_loop = 0;

// Get settings values
String getSettingsValues() {
  JSONVar settingsValues;

  settingsValues["brightnessSlider"] = String(settings->getBrightness());
  settingsValues["switchDialect"] = "false";
  if (settings->getUseDialect() == true) {
    settingsValues["switchDialect"] = "true";
  }

  String jsonString = JSON.stringify(settingsValues);
  return jsonString;
}

TIME getTimeRtc() {
  TIME time;
  DateTime now{rtc.now()};
  time.hour = now.hour();
  time.minute = now.minute();
  time.seconds = now.second();
  time.year = now.year();
  time.month = now.month();
  time.day = now.day();
  time.valid = true;
  return time;
}

TIME getTimeNtp() {
  TIME time;
  timeClient.update();
  time.hour = timeClient.getHours();
  time.minute = timeClient.getMinutes();
  time.seconds = timeClient.getSeconds();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  time.year = ptm->tm_year + 1900;
  time.month = ptm->tm_mon + 1;
  time.day = ptm->tm_mday;
  time.valid = true;
  return time;
}

TIME getTime() {
  TIME time;
  memset(&time, 0, sizeof(time));
  if (WiFi.status() != WL_CONNECTED && rtcValid) {
    return getTimeRtc();
  } else if (WiFi.status() == WL_CONNECTED) {
    return getTimeNtp();
  }
  time.valid = false;
  return time;
}

double calc_scale(u_int16_t active_leds) {
  if (active_leds == 0) {
    active_leds = NUMPIXELS;
  }
  double current_per_color = max_current_ma / (double)active_leds / 3.0;
  return current_per_color / max_current_per_color_ma;
}

/**
 * European daylight savings time calculation by "jurs" from german arduino
 * forum. Idea from https://forum.arduino.cc/t/rtc-mit-sommerzeit/168068/2
 * Daylight saving time is active from last sunday in march till last sunday in
 * october
 *
 * @param epochTime Time in seconds since 01.01.1900
 * @param tzHours Time offset in hours from UTC time
 *
 * @return true when daylight time is active, false if not
 */
boolean summertime_EU(TIME time, s8_t tzHours) {
  u_int16_t year = time.year;
  u_int8_t month = time.month;
  u_int8_t day = time.day;
  u_int8_t hour = time.hour;

  if (month < 3 || month > 10) {
    return false;
  }
  if (month > 3 && month < 10) {
    return true;
  }

  return (month == 3 &&
          (hour + 24 * day) >=
              (1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7))) ||
         (month == 10 &&
          (hour + 24 * day) <
              (1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7)));
}

void initWebFunctions() {
  server.begin();
  Serial.println("HTTP server started");

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

bool updateRtcTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Missing wifi connection, cannot adjust NTP time");
    return false;
  }

  TIME time = getTimeNtp();
  Serial.println("Adjust RTC time from NTP time");
  rtc.adjust(DateTime(time.year, time.month, time.day, time.hour, time.minute,
                      time.seconds));
  return true;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
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
      Serial.print(getSettingsValues());
      notifyClients(getSettingsValues());
    }
    if (message.indexOf("Dialect") == 0) {
      String strDialect = message.substring(message.indexOf("=") + 1);
      settings->setUseDialect(strDialect == "true");
      timeProcessor->setDialect(settings->getUseDialect());
      Serial.println(strDialect);
      Serial.print(getSettingsValues());
      notifyClients(getSettingsValues());
    }

    if (strcmp((char *)data, "getValues") == 0) {
      notifyClients(getSettingsValues());
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

void setup() {
  TIME time;

  Serial.begin(9600);
  delay(1000);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(settings->getBrightness() * calc_scale(NUMPIXELS));
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  // wifiManager.setConfigPortalBlocking(false);

  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
    wifiConnected = true;
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    rtcFound = false;
    rtcValid = false;
  }

  if (rtcFound && rtc.lostPower()) {
    rtcValid = false;
  }

  initFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  if (WiFi.status() == WL_CONNECTED) {
    initWebFunctions();
    time = getTime();
    Serial.println("Set summertime offset");
    if (summertime_EU(time, hourOffsetFromUTC)) {
      timeClient.setTimeOffset((hourOffsetFromUTC + 1) * 3600);
    } else {
      timeClient.setTimeOffset(hourOffsetFromUTC * 3600);
    }
    timeClient.update();
  }

  if (rtcFound && rtc.lostPower()) {
    Serial.println("RTC lost power");
    rtcValid = updateRtcTime();
  }

  matrix.setBrightness(100 * calc_scale(NUMPIXELS));
  matrix.fillRect(0, 0, COL_PIXELS, ROW_PIXELS, matrix.Color(255, 0, 0));
  matrix.show();
  delay(3000);
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
  error = false;

  // wifiManager.process();
  if (!wifiConnected && WiFi.status() == WL_CONNECTED) {
    initWebFunctions();
  }
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    stopWebFunctions();
  }
  wifiConnected = WiFi.status() == WL_CONNECTED;

  TIME time = getTime();
  if (!time.valid) {
    error = true;
  }

  if (time.valid &&
      !timeProcessor->update(time.hour, time.minute, time.seconds)) {
    Serial.println("Error occured");
    error = true;
  }

  if (WiFi.status() == WL_CONNECTED &&
      millis() - lastDaylightCheck > checkDaylightTime) {
    if (summertime_EU(time, hourOffsetFromUTC)) {
      timeClient.setTimeOffset((hourOffsetFromUTC + 1) * 3600);
    } else {
      timeClient.setTimeOffset((hourOffsetFromUTC)*3600);
    }
    time = getTime();
    lastDaylightCheck = millis();
  }

  if (millis() - lastRun > evalTimeEvery) {
    lastRun = millis();
    Serial.println("===========");
    TIME ntpTime = getTimeNtp();
    Serial.print(ntpTime.hour);
    Serial.print(":");
    Serial.print(ntpTime.minute);
    Serial.print(":");
    Serial.println(ntpTime.seconds);
    TIME rtcTime = getTimeRtc();
    Serial.print(rtcTime.hour);
    Serial.print(":");
    Serial.print(rtcTime.minute);
    Serial.print(":");
    Serial.println(rtcTime.seconds);
  }

  if (millis() - lastRtcSync > syncRtc && rtcFound) {
    rtcValid = updateRtcTime();
    lastRtcSync = millis();
  }

  color = matrix.Color(0, 255, 0);
  if (error) {
    color = matrix.Color(255, 0, 0);
  }

  if (millis() - lastTimeUpdate > updateTime && !error) {
    Timestack *stack = timeProcessor->getStack();
    TIMESTACK elem;
    int buffer[4];

    matrix.fillScreen(0);
    active_leds_loop = 0;
    for (int i = 0; i < stack->getSize(); ++i) {
      if (!stack->get(&elem, i)) {
        error = true;
        return;
      }
      get_led_rectangle(elem.state, elem.useDialect, buffer);
      matrix.fillRect(buffer[0], buffer[1], buffer[2], buffer[3], color);
      active_leds_loop += (buffer[2] * buffer[3]);
    }
    lastTimeUpdate = millis();
  }

  matrix.setBrightness(settings->getBrightness() *
                       calc_scale(active_leds_loop));
  matrix.show();

  delay(10);
}
