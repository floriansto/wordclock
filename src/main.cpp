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

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Wire.h>
#include <math.h>
#include <timeprocessor.h>

#include "../include/hw_settings.h"
#include "../include/main.h"
#include "../include/settings.h"
#include "../include/timeUtils.h"
#include "LittleFS.h"

#define DEBUG 0

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
DynamicJsonDocument wordsDoc(8192);
JsonObject words;

TimeProcessor *timeProcessor = new TimeProcessor(
    settings->getUseDialect(), settings->getUseQuaterPast(),
    settings->getUseThreeQuater(), offsetLowSecs, offsetHighSecs, NUMPIXELS);

Error error = Error::OK;
bool wifiConnected = false;
u_int16_t numActiveLeds = NUMPIXELS;

double calcBrightnessScale(u_int16_t activeLeds) {
  double maxCurrent = maxCurrentPerLed * (double)activeLeds;
  if (maxCurrent < maxCurrentAll) {
    return 255.0;
  }
  return 255.0 * maxCurrentAll / maxCurrent;
}

void getWordCoords(int *buffer, String wordKey, String langKey) {
  JsonArray list;
  u_int8_t j;

  list = words[wordKey][langKey]["coords"].as<JsonArray>();

  j = 0;
  for (JsonVariant v : list) {
    buffer[j++] = v.as<int>();
  }
}

void showTime(COLOR color, COLOR background) {
  Timestack *stack = timeProcessor->getStack();
  TIMESTACK elem;
  JsonArray list;
  int buffer[4]{0, 0, 0, 0};
  String langKey;
  String wordKey;

  matrix.fillScreen(0);

  if (settings->getUseDialect())
    langKey = "de-Dialect";
  else
    langKey = "de-DE";

  if (settings->getUseBackgroundColor() == true) {
    matrix.fillRect(0, 0, COL_PIXELS, ROW_PIXELS,
                    matrix.Color(background.r, background.g, background.b));
  }
  numActiveLeds = 0;
  for (int i = 0; i < stack->getSize(); ++i) {
    if (!stack->get(&elem, i)) {
      error = Error::TIMESTACK_GET_ELEM_FAILED;
      return;
    }

    json_key_from_state(elem.state);
    getWordCoords(buffer, wordKey, langKey);

    matrix.fillRect(buffer[0], buffer[1], buffer[2], buffer[3],
                    matrix.Color(color.r, color.g, color.b));

    numActiveLeds += (buffer[2] * buffer[3]);
  }

  if (settings->getUseBackgroundColor() == true) {
    numActiveLeds = NUMPIXELS;
  }

  matrix.setBrightness(settings->getBrightness() / 100.0 *
                       calcBrightnessScale(numActiveLeds));
  matrix.show();
}

void updateSettings() {
  timeProcessor->setDialect(settings->getUseDialect());
  timeProcessor->setThreeQuater(settings->getUseThreeQuater());
  timeProcessor->setQuaterPast(settings->getUseQuaterPast());
  timeProcessor->update();
  matrix.setBrightness(settings->getBrightness() / 100.0 *
                       calcBrightnessScale(numActiveLeds));
}

void getActiveLeds(u_int16_t *dest) {
  Timestack *stack = timeProcessor->getStack();
  TIMESTACK elem;
  JsonArray list;
  int buffer[4]{0, 0, 0, 0};
  String langKey;
  String wordKey;
  u_int8_t x;
  u_int8_t y;
  u_int8_t x_len;
  u_int8_t y_len;

  if (settings->getUseDialect())
    langKey = "de-Dialect";
  else
    langKey = "de-DE";

  /* Raise datatype of dest to u_int32_t to handle more
  than 16 columns of leds. */
  static_assert(COL_PIXELS <= 16);

  for (u_int8_t k = 0; k < stack->getSize(); ++k) {
    if (!stack->get(&elem, k)) {
      error = Error::TIMESTACK_GET_ELEM_FAILED;
      return;
    }

    json_key_from_state(elem.state);
    getWordCoords(buffer, wordKey, langKey);

    x = buffer[0];
    y = buffer[1];
    x_len = buffer[2];
    y_len = buffer[3];

    for (u_int8_t j = y; j < y + y_len; ++j) {
      for (u_int8_t i = x; i < x + x_len; ++i) {
        dest[j] |= 1 << i;
      }
    }
  }
}

String getWordTime() {
  TIMESTACK elem;
  String langKey;
  String wordKey;
  Timestack *stack = timeProcessor->getStack();
  String wordTime{""};

  if (settings->getUseDialect())
    langKey = "de-Dialect";
  else
    langKey = "de-DE";

  for (int i = 0; i < stack->getSize(); ++i) {
    if (!stack->get(&elem, i)) {
      error = Error::TIMESTACK_GET_ELEM_FAILED;
      wordTime = "Error";
      break;
    }
    json_key_from_state(elem.state);

    wordTime += words[wordKey][langKey]["name"].as<String>();
    wordTime += " ";
  }
  return wordTime;
}

void notifyClients() {
  StaticJsonDocument<JSON_SETTINGS_SIZE> json;

  updateSettings();

  settings->toJsonDoc(json);
  settings->saveSettings(json);
}

JsonArray getActiveLedsToWeb(JsonDocument &json) {
  String jsonString;
  u_int16_t activeLeds[ROW_PIXELS];
  memset(activeLeds, 0, sizeof(activeLeds));

  getActiveLeds(activeLeds);

  JsonArray array = json.createNestedArray("activeLeds");

  for (u_int8_t i = 0; i < ROW_PIXELS; ++i) {
    array.add(activeLeds[ROW_PIXELS - 1 - i]);
  }

  return array;
}

void getTimeToWeb(JsonDocument &json) {
  json["wordTime"] = getWordTime();
  getActiveLedsToWeb(json);
}

void getSettingsToWeb(JsonDocument &json) { settings->toJsonDoc(json); }

void sendJson(void function(JsonDocument &json)) {
  StaticJsonDocument<JSON_SETTINGS_SIZE> json;
  String str;

  (*function)(json);

  if (serializeJson(json, str) == 0) {
    Serial.println("Failed to serialize json");
    return;
  }
  ws.textAll(str);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char *)data;
    if (message.indexOf("Brightness") == 0) {
      int brightness = message.substring(message.indexOf("=") + 1).toInt();
      settings->setBrightness(brightness);
      notifyClients();
    }
    if (message.indexOf("switchDialect") == 0) {
      int dialect = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseDialect(dialect > 0);
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("switchThreeQuater") == 0) {
      int threeQuater = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseThreeQuater(threeQuater > 0);
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("switchQuaterPast") == 0) {
      int quaterPast = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseQuaterPast(quaterPast > 0);
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("switchBackgroundColor") == 0) {
      int quaterPast = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseBackgroundColor(quaterPast > 0);
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("mainColor") == 0) {
      String mainColorStr = message.substring(message.indexOf("=") + 1);
      char colorChar[9];
      mainColorStr.toCharArray(colorChar, 9);
      int mainColor = strtol(colorChar, 0, 16);

      uint8_t r = (mainColor & 0xFF0000) >> 16;
      uint8_t g = (mainColor & 0x00FF00) >> 8;
      uint8_t b = mainColor & 0x0000FF;
      settings->setMainColor(COLOR{r, g, b});
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("backgroundColor") == 0) {
      String backgroundColorStr = message.substring(message.indexOf("=") + 1);
      char colorChar[9];
      backgroundColorStr.toCharArray(colorChar, 9);
      int backgroundColor = strtol(colorChar, 0, 16);

      uint8_t r = (backgroundColor & 0xFF0000) >> 16;
      uint8_t g = (backgroundColor & 0x00FF00) >> 8;
      uint8_t b = backgroundColor & 0x0000FF;
      settings->setBackgroundColor(COLOR{r, g, b});
      notifyClients();
      sendJson(getTimeToWeb);
    }
    if (message.indexOf("wordConfig") == 0) {
      String wordConfig = message.substring(message.indexOf("=") + 1);
      Serial.println(wordConfig);
    }

    if (strcmp((char *)data, "getValues") == 0) {
      sendJson(getSettingsToWeb);
    }
    if (strcmp((char *)data, "getTime") == 0) {
      sendJson(getTimeToWeb);
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

void loadWordConfig() {
  File file = LittleFS.open("/words.json", "r");
  if (!file) {
    Serial.println("words.json not found!");
    return;
  }
  DeserializationError error = deserializeJson(wordsDoc, file);
  if (error) {
    file.close();
    Serial.println("Failed to read words.json using default configuration");
    Serial.println(error.f_str());
    return;
  }
  file.close();

  words = wordsDoc.as<JsonObject>();
  for (JsonObject::iterator it = words.begin(); it != words.end(); ++it) {
    if (!words[it->key()].containsKey("de-DE")) {
      words.remove(it);
      continue;
    }

    if (!words[it->key()].containsKey("de-Dialect"))
      words[it->key()]["de-Dialect"] = words[it->key()]["de-DE"];
  }
#if DEBUG
  serializeJsonPretty(words, Serial);
#endif
}

void setup() {
  Serial.begin(9600);

  Wire.begin(D2, D1);

  matrix.begin();
  matrix.setTextWrap(false);

  initFS();

  settings->loadSettings();
  loadWordConfig();
  updateSettings();

  rtc.found = true;
  rtc.valid = true;
  if (!rtc.rtc.begin()) {
    Serial.println("Couldn't find RTC");
    rtc.found = false;
  }

  if (rtc.found == true && rtc.rtc.lostPower()) {
    rtc.valid = false;
  }

  TIME time = getTime(&rtc, &timeClient, wifiConnected);
  if (time.valid == true &&
      timeProcessor->update(time.hour, time.minute, time.seconds) == true) {
    showTime(settings->getMainColor(), settings->getBackgroundColor());
  }

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(120);
  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
    wifiConnected = true;
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
  }

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
u_int32_t checkDaylightTime = 3600000;
unsigned long lastRtcSync = 0;
u_int32_t syncRtc = 24 * 3600;
unsigned long lastTimeUpdate = 0;
u_int32_t updateTime = 1 * 1000;

bool showCaptivePortal = true;

void loop() {
  error = Error::OK;

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

#if DEBUG == 1
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
#endif

  if (millis() - lastRtcSync > syncRtc && rtc.found == true) {
    if (updateRtcTime(&rtc, &time, wifiConnected) == false) {
      error = Error::UPDATE_RTC_TIME_ERROR;
    }
    lastRtcSync = millis();
  }

  if (millis() - lastTimeUpdate > updateTime && time.valid == true) {
    showTime(settings->getMainColor(), settings->getBackgroundColor());
    lastTimeUpdate = millis();
  }

  delay(10);
}
