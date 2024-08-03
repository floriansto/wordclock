#include <Arduino.h>
#include <FastLED.h>

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

#include "../include/color.h"
#include "../include/hw_settings.h"
#include "../include/main.h"
#include "../include/settings.h"
#include "../include/timeUtils.h"
#include "../include/wordConfig.h"
#include "LittleFS.h"

#define DEBUG 0

CRGB leds[NUMPIXELS];
CRGB oldColor[NUMPIXELS];
CRGB newColor[NUMPIXELS];
double interpolationTime[NUMPIXELS];

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncWebSocket ws("/ws");

WiFiUDP ntpUDP;
RTC rtc;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

Settings *settings = new Settings(LedWiring::ZIGZAG);
WORD words[MAX_WORDS];

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

#if DEBUG
void printColor(CRGB *color) {
  Serial.print("rgb: (");
  Serial.print(color->r);
  Serial.print(", ");
  Serial.print(color->g);
  Serial.print(", ");
  Serial.print(color->b);
  Serial.println(")");
}
#endif

/**
 * Map the led number from the configuration file to the
 * led index on the led strip.
 * The led index on the strip depends on the wiring of
 * the led strips.
 * The leds in the configuration file are numbered like you
 * read a book: Top left is zero, after the first row begin
 * the next row on the left again.
 */
u_int16_t mapLedIndex(u_int16_t led) {
  /* Map led index depending on the wiring of the led strip*/
  switch (settings->getLedWiring()) {
  case LedWiring::ZIGZAG:
    if ((int(led / COL_PIXELS)) % 2 == 0)
      return led;
    return COL_PIXELS - (led % COL_PIXELS) +
           COL_PIXELS * (int)(led / COL_PIXELS) - 1;
  default:
    return led;
  }
}

/**
 * Set new target and start led colors for a given time
 */
void setLeds() {
  u_int32_t timeColor = rgbToHex(settings->getTimeColor());
  u_int32_t background = rgbToHex(settings->getBackgroundColor());
  Timestack *stack = timeProcessor->getStack();
  TIMESTACK elem;
  LANGUAGE langKey;
  JsonArray wordPixels;
  u_int16_t led;
  CRGB timeColorRgb = timeColor;
  CRGB backgroundRgb = background;
  CRGB customColorRgb;
  CRGB targetColor[NUMPIXELS];
  bool timeLeds[NUMPIXELS];
  bool customColor[NUMPIXELS];
  bool showTime = true;
  TIME time;
  WordConfig* wordConfig;

  langKey = settings->getLangKey();
  memset(timeLeds, false, sizeof(timeLeds));
  memset(customColor, false, sizeof(customColor));
  memset(targetColor, 0, sizeof(targetColor));

  wordConfig = settings->getWordConfig();
  for (uint8_t k = 0; k < settings->getMaxWordConfigs(); ++k) {
    if (!wordConfig[k].isEnabled()) {
      continue;
    }
    if (!wordConfig[k].showTime()) {
      showTime = false;
    }
  }

  if (showTime) {
    for (u_int16_t i = 0; i < stack->getSize(); ++i) {
      /* Get the word key for each stack element */
      if (!stack->get(&elem, i)) {
        error = Error::TIMESTACK_GET_ELEM_FAILED;
        return;
      }

      for (uint8_t j = 0; j < MAX_WORDS; ++j) {
        if (words[j].type != elem.state) {
          continue;
        }
        /* Get the active pixels for the current word */
        for (uint8_t k = 0; k < words[j].properties[langKey].numPixels; ++k) {
          led = mapLedIndex(words[j].properties[langKey].pixels[k]);
          /* Set the color for each active pixel */
          timeLeds[led] = true;
          if (newColor[led] != timeColorRgb) {
            oldColor[led] = leds[led];
            interpolationTime[led] = 0.0;
          }
          newColor[led] = timeColor;
        }
        break;
      }
    }
  }

  time = getTime(&rtc, &timeClient, wifiConnected);
  for (uint8_t k = 0; k < settings->getMaxWordConfigs(); ++k) {
    if (!wordConfig[k].isEnabled()) {
      continue;
    }

    for (uint16_t i = 0; i < MAX_LED_ENTRIES; ++i) {
      for (uint16_t j = 0; j < BITMASK_LENGTH; ++j)
      {
        if (((1 << j) & wordConfig[k].getLedsAt(i)) == 0) {
          continue;
        }
        led = mapLedIndex(j + (BITMASK_LENGTH * i));
        if (timeLeds[led]) {
          continue;
        }
        customColor[led] = true;
        customColorRgb = wordConfig[k].getColor();
        targetColor[led] = rgbToHex(customColorRgb);
      }
    }
  }

  for (u_int16_t i = 0; i < NUMPIXELS; ++i) {
    if (!customColor[i]) {
      continue;
    }
    if (targetColor[i] == newColor[i]) {
      continue;
    }
    oldColor[i] = leds[i];
    newColor[i] = targetColor[i];
    interpolationTime[i] = 0.0;
  }

  /* Fill all non time relevant leds with the background color */
  for (u_int16_t i = 0; i < NUMPIXELS; ++i) {
    if (timeLeds[i] || customColor[i]) {
      continue;
    }
    if (newColor[i] == backgroundRgb) {
      continue;
    }
    newColor[i] = backgroundRgb;
    oldColor[i] = leds[i];
    interpolationTime[i] = 0.0;
  }
}

void interpolateLeds() {
  u_int16_t interpolationDuration = 2000;
  CRGB prevCol1{0, 0, 0};
  CRGB prevCol2{0, 0, 0};
  u_int32_t c = 0x0;
  bool equal{false};
  double t;
  for (u_int16_t i = 0; i < NUMPIXELS; ++i) {
    if (interpolationTime[i] > interpolationDuration) {
      continue;
    }
    if (prevCol1 != oldColor[i]) {
      prevCol1 = oldColor[i];
      equal = false;
    }
    if (prevCol2 != newColor[i]) {
      prevCol2 = newColor[i];
      equal = false;
    }
    if (!equal) {
      t = interpolationTime[i] / interpolationDuration;
      c = rgb_interp(rgbToHex(oldColor[i]), rgbToHex(newColor[i]), t);
    }
    leds[i] = CRGB{c};
    interpolationTime[i] += cycleTimeMs;
    equal = true;
  }
}

void updateSettings() {
  timeProcessor->setDialect(settings->getUseDialect());
  timeProcessor->setThreeQuater(settings->getUseThreeQuater());
  timeProcessor->setQuaterPast(settings->getUseQuaterPast());
  timeProcessor->update();
  FastLED.setBrightness(settings->getBrightness() / 100.0 *
                        calcBrightnessScale(numActiveLeds));
}

String getWordTime() {
  TIMESTACK elem;
  LANGUAGE langKey;
  Timestack *stack = timeProcessor->getStack();
  String wordTime{""};

  langKey = settings->getLangKey();

  for (uint8_t i = 0; i < stack->getSize(); ++i) {
    if (!stack->get(&elem, i)) {
      error = Error::TIMESTACK_GET_ELEM_FAILED;
      wordTime = "Error";
      break;
    }

    for (uint8_t j = 0; j < MAX_WORDS; ++j) {
      if (words[j].type != elem.state) {
        continue;
      }
      wordTime += words[j].properties[langKey].name;
      wordTime += " ";
      break;
    }
  }
  return wordTime;
}

void notifyClients() {
  updateSettings();
  settings->saveSettings();
  setLeds();
}

void serializeLedColor(JsonDocument &json) {
  String jsonString;

  JsonArray array = json.createNestedArray("activeLeds");

  for (u_int8_t i = 0; i < NUMPIXELS; ++i) {
    array.add(rgbToHex(newColor[mapLedIndex(i)]));
  }
}

void sendMessage(JsonDocument &json) {
  String str;

  if (serializeJson(json, str) == 0) {
    Serial.println("Failed to serialize json");
    return;
  }
  ws.textAll(str);
}

void updateTimeOnWeb() {
  DynamicJsonDocument json(4096);
  //StaticJsonDocument<128> json;
  json["wordTime"] = getWordTime();
  serializeLedColor(json);
  sendMessage(json);
}

void continueWordConfig() {
  StaticJsonDocument<32> json;
  json["continueWordConfig"] = true;
  sendMessage(json);
}

void sendSettingsToWeb() {
  StaticJsonDocument<512> json;
  JsonObject obj = json.to<JsonObject>();
  settings->serializeBasic(obj);
  sendMessage(json);
}

void sendWordConfigToWeb(uint8_t index) {
  if (index < settings->getMaxWordConfigs()) {
    StaticJsonDocument<512> json;
    JsonObject obj = json["wordConfig"].to<JsonObject>();
    settings->getWordConfig()[index].serialize(obj);
    obj["index"] = index;
    sendMessage(json);
  } else {
    StaticJsonDocument<64> json;
    json["wordConfigNumRows"] = settings->getMaxWordConfigs();
    sendMessage(json);
  }
}


bool updateTime(TIME* time) {
  *time = getTime(&rtc, &timeClient, wifiConnected);

  /* Error if time is not valid */
  if (!time->valid) {
    error = Error::NO_TIME;
    return false;
  }

  /* Run the wordprocessor to update the wordtime */
  if (!timeProcessor->update(time->hour, time->minute, time->seconds)) {
    error = Error::TIME_TO_WORD_CONVERSION;
    return false;
  }
  return true;
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
    if (message.indexOf("useDialect") == 0) {
      int dialect = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseDialect(dialect > 0);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("useThreeQuater") == 0) {
      int threeQuater = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseThreeQuater(threeQuater > 0);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("useQuaterPast") == 0) {
      int quaterPast = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseQuaterPast(quaterPast > 0);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("useBackgroundColor") == 0) {
      int useBackgroundColor =
          message.substring(message.indexOf("=") + 1).toInt();
      settings->setUseBackgroundColor(useBackgroundColor > 0);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("backgroundColor") == 0) {
      String backgroundColorStr = message.substring(message.indexOf("=") + 1);
      settings->setBackgroundColor(backgroundColorStr);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("timeColor") == 0) {
      String timeColorStr = message.substring(message.indexOf("=") + 1);
      settings->setTimeColor(timeColorStr);
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("utcTimeOffset") == 0) {
      int offset = message.substring(message.indexOf("=") + 1).toInt();
      settings->setUtcHourOffset(offset);
      if (adjustSummertime(&rtc, &timeClient, offset, wifiConnected) != true) {
        Serial.println("Failed to adjust time");
        error = Error::SUMMERTIME_ERROR;
      }
      TIME time;
      if (updateTime(&time) == false) {
        return;
      }
      notifyClients();
      updateTimeOnWeb();
    }
    if (message.indexOf("wordConfig") == 0) {
      String wordConfig = message.substring(message.indexOf("=") + 1);
      Serial.println("Set word config");
      settings->setWordConfig(wordConfig);
      continueWordConfig();
    }
    if (message.indexOf("finishedWordConfig") == 0) {
      notifyClients();
      settings->saveWordConfig();
      sendWordConfigToWeb(0);
      updateTimeOnWeb();
    }
    if (message.indexOf("clearWordConfig") == 0) {
      Serial.println("Clear word config");
      settings->clearWordConfig();
    }
    if (message.indexOf("continueSendWordConfig") == 0) {
      uint8_t index = message.substring(message.indexOf("=") + 1).toInt();
      sendWordConfigToWeb(index);
    }

    if (strcmp((char *)data, "getValues") == 0) {
      sendSettingsToWeb();
      sendWordConfigToWeb(0);
    }
    if (strcmp((char *)data, "getTime") == 0) {
      updateTimeOnWeb();
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

LANGUAGE getLanguageKey(const char* name) {
  map_lang_string_to_enum
  return MAX_LANGUAGES;
}

void loadWordConfig() {
  File file = LittleFS.open("/words.json", "r");
  uint8_t i{0};
  uint8_t k{0};
  LANGUAGE lang;
  if (!file) {
    Serial.println("words.json not found!");
    return;
  }
  file.find("\"words\": [");
  do {
    StaticJsonDocument<1024> wordsDoc;
    DeserializationError error = deserializeJson(wordsDoc, file);
    if (error) {
      file.close();
      Serial.println("Failed to read words.json using default configuration");
      Serial.println(error.f_str());
      break;
    }
    JsonObject obj = wordsDoc.as<JsonObject>();

    for (JsonPair kv : obj) {
      JsonObject value = kv.value().as<JsonObject>();
      if (!value.containsKey("de-DE")) {
        continue;
      }
      if (!value.containsKey("de-Dialect")) {
        value["de-Dialect"] = value["de-DE"];
      }
      words[i].type = getStateFromName(kv.key().c_str());
      for (JsonPair props : kv.value().as<JsonObject>()) {
        JsonObject langProperties = props.value().as<JsonObject>();
        lang = getLanguageKey(props.key().c_str());
        words[i].properties[lang].name = langProperties["name"].as<String>();
        k = 0;
        for (JsonVariant j : langProperties["pixels"].as<JsonArray>()) {
          words[i].properties[lang].pixels[k++] = j.as<uint16_t>();
        }
        words[i].properties[lang].numPixels = k;
      }
    }
    ++i;
  } while (file.findUntil(",","]"));

  file.close();

#if DEBUG
  serializeJsonPretty(words, Serial);
#endif
}

void setup() {
  /* Hardware setup */
  Serial.begin(9600);
  Wire.begin(D2, D1);

  /* Setup ledsrip */
  FastLED.addLeds<NEOPIXEL, PIN>(leds, NUMPIXELS);
  memset(leds, 0, sizeof(leds));
  memset(oldColor, 0, sizeof(oldColor));
  memset(newColor, 0, sizeof(newColor));
  FastLED.setBrightness(64);
  FastLED.clear();
  FastLED.show();

  memset(interpolationTime, 0, sizeof(interpolationTime));

  /* Initialize filesystem */
  initFS();

  settings->loadSettings();
  settings->loadWordConfig();
  loadWordConfig();
  updateSettings();

  /* Start connection to rtc if possible */
  rtc.found = true;
  rtc.valid = true;
  if (!rtc.rtc.begin()) {
    Serial.println("Couldn't find RTC");
    rtc.found = false;
  }

  /* If the rtc lost its power, mark the time as invalid */
  if (rtc.found == true && rtc.rtc.lostPower()) {
    rtc.valid = false;
  }

  /* Initialize wifi connection or enable the hotspot */
  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(120);
  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
  }
  setLeds();
  FastLED.show();
}

/* Check summertime every hour */
u_int32_t checkDaylightTime = 3600 * 1000;
unsigned long lastDaylightCheck = checkDaylightTime;

/* Update rtc time every day */
u_int32_t syncRtc = 24 * 3600 * 1000;
unsigned long lastRtcSync = syncRtc;

/* Update time every second */
u_int32_t updateTimeInterval = 1 * 1000;
unsigned long lastTimeUpdate = updateTimeInterval;

void loop() {

  TIME time;
  time.valid = false;

  /* Starttimer */
  unsigned long start = millis();

  /* Display all red leds in red in case of an error. */
  if (error != Error::OK) {
    for (u_int8_t i = 0; i < NUMPIXELS; ++i) {
      leds[i] = 0xFF0000;
    }
    FastLED.show();
    Serial.println("Error detected");
    return;
  }

  /* Start webserver and ntp time when wifi was not connected during
  setup and connection is now available. */
  if (!wifiConnected && WiFi.status() == WL_CONNECTED) {
    initWebFunctions();
  }
  /* Stop webserver and ntp time when wifi conneciton is lost */
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    stopWebFunctions();
  }
  /* Store state of wifi connection */
  wifiConnected = WiFi.status() == WL_CONNECTED;

  /* Get the current time */
  if (millis() - lastTimeUpdate > updateTimeInterval) {
    if (updateTime(&time) == false) {
      return;
    }
    lastTimeUpdate = millis();
  }

  /* Check if the summertime needs to be adjusted and if so, do so */
  if (millis() - lastDaylightCheck > checkDaylightTime) {
    if (adjustSummertime(&rtc, &timeClient, settings->getUtcHourOffset(),
                         wifiConnected) != true) {
      error = Error::SUMMERTIME_ERROR;
      return;
    }
    lastDaylightCheck = millis();
  }

  /* Update the time on the rtc from the ntp time */
  if (millis() - lastRtcSync > syncRtc && rtc.found == true) {
    if (updateRtcTime(&rtc, &time, wifiConnected) == false) {
      error = Error::UPDATE_RTC_TIME_ERROR;
      return;
    }
    lastRtcSync = millis();
  }

  setLeds();
  interpolateLeds();
  FastLED.show();

  unsigned long end = millis();

  if (cycleTimeMs > end - start) {
    delay(cycleTimeMs - end + start);
  }
}
