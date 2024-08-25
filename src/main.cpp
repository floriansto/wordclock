#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

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

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

COLOR_RGB oldColor[NUMPIXELS];
COLOR_RGB newColor[NUMPIXELS];
double interpolationTime[NUMPIXELS];

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncWebSocket ws("/ws");

WiFiUDP ntpUDP;
RTC rtc;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

Settings setting = Settings();
Settings *settings = &setting;

WORD words[MAX_WORDS];

TimeProcessor *timeProcessor = new TimeProcessor(
    settings->getUseDialect(), settings->getUseQuaterPast(),
    settings->getUseThreeQuater(), offsetLowSecs, offsetHighSecs, NUMPIXELS);

Error error = Error::OK;
bool wifiConnected = false;
u_int16_t numActiveLeds = NUMPIXELS;
char startTime[30];

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

uint16_t mirrorLedHorizontal(uint16_t led) {
  return COL_PIXELS - (led % COL_PIXELS) +
         COL_PIXELS * (uint16_t)(led / COL_PIXELS) - 1;
}

uint16_t mirrorLedVertical(uint16_t led) {
  uint16_t rowsAbove = ROW_PIXELS - (uint16_t)(led / COL_PIXELS) - 1;
  uint16_t ledInRow = COL_PIXELS - (led % COL_PIXELS);
  if (rowsAbove % 2 == 1) {
    ledInRow = COL_PIXELS - (led % COL_PIXELS) - 1;
  } else {
    ledInRow = led % COL_PIXELS;
  }
  return rowsAbove * COL_PIXELS + ledInRow;
}

/**
 * Map the led number from the configuration file to the
 * led index on the led strip.
 * The led index on the strip depends on the wiring of
 * the led strips.
 * The leds in the configuration file are numbered like you
 * read a book: Top left is zero, after the first row begin
 * the next row on the left again.
 */
uint16_t mapLedIndex(uint16_t led) {
  /* Map led index depending on the wiring of the led strip*/
  switch (firstLedPosition) {
  case FirstLedPosition::TopLeft:
    if ((uint8_t(led / COL_PIXELS)) % 2 == 0) {
      return led;
    }
    return mirrorLedHorizontal(led);
  case FirstLedPosition::TopRight:
    if ((uint8_t(led / COL_PIXELS)) % 2 == 1) {
      return led;
    }
    return mirrorLedHorizontal(led);
  case FirstLedPosition::BottomLeft:
    return mirrorLedVertical(led);
  case FirstLedPosition::BottomRight:
    return mirrorLedHorizontal(mirrorLedVertical(led));
  default:
    return led;
  }
}

/**
 * Set new target and start led colors for a given time
 */
void setLeds() {
  COLOR_RGB timeColor = settings->getTimeColor();
  COLOR_RGB background = settings->getBackgroundColor();
  Timestack *stack = timeProcessor->getStack();
  TIMESTACK elem;
  LANGUAGE langKey;
  JsonArray wordPixels;
  u_int16_t led;
  COLOR_RGB timeColorRgb = timeColor;
  COLOR_RGB backgroundRgb = background;
  COLOR_RGB customColorRgb;
  COLOR_RGB targetColor[NUMPIXELS];
  bool timeLeds[NUMPIXELS];
  bool customColor[NUMPIXELS];
  bool showTime = true;
  TIME time;
  WordConfig *wordConfig;

  langKey = settings->getLangKey();
  memset(timeLeds, false, sizeof(timeLeds));
  memset(customColor, false, sizeof(customColor));
  memset(targetColor, 0, sizeof(targetColor));

  time = getTime(&rtc, &timeClient, wifiConnected);
  wordConfig = settings->getWordConfig();
  for (uint8_t k = 0; k < settings->getMaxWordConfigs(); ++k) {
    if (!wordConfig[k].isWordConfigActive(time.day, time.month)) {
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
        for (uint8_t k = 0; k < words[j].properties[langKey].length; ++k) {
          led = mapLedIndex(k + words[j].properties[langKey].startPixel);
          /* Set the color for each active pixel */
          timeLeds[led] = true;
          if (newColor[led] != timeColorRgb) {
            oldColor[led] = pixels.getPixelColor(led);
            interpolationTime[led] = 0.0;
          }
          newColor[led] = timeColor;
        }
      }
    }
  }

  for (uint8_t k = 0; k < settings->getMaxWordConfigs(); ++k) {
    if (!wordConfig[k].isWordConfigActive(time.day, time.month)) {
      continue;
    }

    for (uint16_t i = 0; i < MAX_LED_ENTRIES; ++i) {
      for (uint16_t j = 0; j < BITMASK_LENGTH; ++j) {
        if (((1 << j) & wordConfig[k].getLedsAt(i)) == 0) {
          continue;
        }
        led = mapLedIndex(j + (BITMASK_LENGTH * i));
        if (timeLeds[led]) {
          continue;
        }
        customColor[led] = true;
        customColorRgb = wordConfig[k].getColor();
        targetColor[led] = customColorRgb;
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
    oldColor[i] = pixels.getPixelColor(i);
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
    oldColor[i] = pixels.getPixelColor(i);
    interpolationTime[i] = 0.0;
  }
}

void interpolateLeds() {
  u_int16_t interpolationDuration = 2000;
  COLOR_RGB prevCol1{0, 0, 0};
  COLOR_RGB prevCol2{0, 0, 0};
  COLOR_RGB c{0, 0, 0};
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
      c = hexToRgb(rgb_interp(rgbToHex(oldColor[i]), rgbToHex(newColor[i]), t));
    }
    pixels.setPixelColor(i, pixels.Color(c.r, c.g, c.b));
    interpolationTime[i] += cycleTimeMs;
    equal = true;
  }
}

void updateSettings() {
  timeProcessor->setDialect(settings->getUseDialect());
  timeProcessor->setThreeQuater(settings->getUseThreeQuater());
  timeProcessor->setQuaterPast(settings->getUseQuaterPast());
  timeProcessor->update();
  pixels.setBrightness(settings->getBrightness() / 100.0 *
                        calcBrightnessScale(numActiveLeds));
}

void getWordTime(char *wordTime, uint8_t maxLen) {
  TIMESTACK elem;
  LANGUAGE langKey;
  Timestack *stack = timeProcessor->getStack();
  char *nextWord;

  langKey = settings->getLangKey();
  memset(wordTime, '\0', sizeof(char) * maxLen);

  for (uint8_t i = 0; i < stack->getSize(); ++i) {
    if (!stack->get(&elem, i)) {
      error = Error::TIMESTACK_GET_ELEM_FAILED;
      strcpy(wordTime, "Error");
      break;
    }

    for (uint8_t j = 0; j < MAX_WORDS; ++j) {
      if (words[j].type != elem.state) {
        continue;
      }
      nextWord = words[j].properties[langKey].name;
      if (strlen(wordTime) + strlen(nextWord) + 2 >= maxLen) {
        return;
      }
      strcat(wordTime, words[j].properties[langKey].name);
      strcat(wordTime, " ");
      break;
    }
  }
}

void notifyClients() {
  updateSettings();
  settings->saveSettings();
  setLeds();
}

void sendMessage(JsonDocument &json) {
  char str[500];

  if (serializeJson(json, str) == 0) {
    Serial.println("Failed to serialize json");
    return;
  }
  ws.textAll(str);
}

void sendStarttime() {
  StaticJsonDocument<64> json;
  json["uptime"] = startTime;
  sendMessage(json);
}

void sendPreviewToWeb(uint8_t startIdx) {
  StaticJsonDocument<JSON_SIZE_PREVIEW> json;
  JsonArray leds = json["activeLeds"].createNestedArray("leds");
  uint16_t i;
  for (i = startIdx; i < startIdx + PREVIEW_LEDS; ++i) {
    if (i >= NUMPIXELS) {
      break;
    }
    char hexColor[7];
    snprintf(hexColor, sizeof(hexColor), "%06X", rgbToHex(newColor[mapLedIndex(i)]));
    leds.add(hexColor);
  }
  json["activeLeds"]["startIdx"] = startIdx;
  json["activeLeds"]["len"] = i - startIdx;
  sendMessage(json);
}

void updateTimeOnWeb() {
  StaticJsonDocument<128> json;
  char wordTime[MAX_WORDTIME_LENGTH];
  getWordTime(wordTime, MAX_WORDTIME_LENGTH);
  json["wordTime"] = wordTime;
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
    StaticJsonDocument<JSON_SIZE_WORD_CONFIG> json;
    JsonObject obj = json.createNestedObject("wordConfig");
    settings->getWordConfig()[index].serialize(obj);
    obj["index"] = index;
    sendMessage(json);
  } else {
    StaticJsonDocument<64> json;
    json["wordConfigNumRows"] = settings->getMaxWordConfigs();
    sendMessage(json);
  }
}

bool updateTime(TIME *time) {
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

MessageId getMessageId(const char *message) {
  char *startAddress;
  startAddress = strstr(message, "Brightness");
  if (startAddress == message) {
    return MessageId::BRIGHTNESS;
  }
  startAddress = strstr(message, "useDialect");
  if (startAddress == message) {
    return MessageId::DIALECT;
  }
  startAddress = strstr(message, "useThreeQuater");
  if (startAddress == message) {
    return MessageId::THREE_QUATER;
  }
  startAddress = strstr(message, "useQuaterPast");
  if (startAddress == message) {
    return MessageId::QUATER_PAST;
  }
  startAddress = strstr(message, "useBackgroundColor");
  if (startAddress == message) {
    return MessageId::USE_BACKGROUND;
  }
  startAddress = strstr(message, "backgroundColor");
  if (startAddress == message) {
    return MessageId::BACKGROUND_COLOR;
  }
  startAddress = strstr(message, "timeColor");
  if (startAddress == message) {
    return MessageId::TIME_COLOR;
  }
  startAddress = strstr(message, "utcTimeOffset");
  if (startAddress == message) {
    return MessageId::UTC_TIME_OFFSET;
  }
  startAddress = strstr(message, "wordConfig");
  if (startAddress == message) {
    return MessageId::WORDCONFIG;
  }
  startAddress = strstr(message, "finishedWordConfig");
  if (startAddress == message) {
    return MessageId::FINISHED_WORDCONFIG;
  }
  startAddress = strstr(message, "clearWordConfig");
  if (startAddress == message) {
    return MessageId::CLEAR_WORDCONFIG;
  }
  startAddress = strstr(message, "continueSendWordConfig");
  if (startAddress == message) {
    return MessageId::CONTINUE_SEND_WORDCONFIG;
  }
  startAddress = strstr(message, "getValues");
  if (startAddress == message) {
    return MessageId::GET_VALUES;
  }
  startAddress = strstr(message, "getTime");
  if (startAddress == message) {
    return MessageId::GET_TIME;
  }
  startAddress = strstr(message, "continueSendPreview");
  if (startAddress == message) {
    return MessageId::CONTINUE_SEND_PREVIEW;
  }
  return MessageId::UNKNOWN;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!(info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT)) {
    return;
  }
  data[len] = 0;
  const char *message = (char *)data;
  char *messageBegin = strstr(message, "=");
  if (messageBegin) {
    ++messageBegin;
  }
  MessageId id = getMessageId(message);
  switch (id) {
  case MessageId::BRIGHTNESS: {
    int brightness = atoi(messageBegin);
    settings->setBrightness(brightness);
    notifyClients();
    break;
  }
  case MessageId::DIALECT: {
    int dialect = atoi(messageBegin);
    settings->setUseDialect(dialect > 0);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::THREE_QUATER: {
    int threeQuater = atoi(messageBegin);
    settings->setUseThreeQuater(threeQuater > 0);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::QUATER_PAST: {
    int quaterPast = atoi(messageBegin);
    settings->setUseQuaterPast(quaterPast > 0);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::USE_BACKGROUND: {
    int useBackgroundColor = atoi(messageBegin);
    settings->setUseBackgroundColor(useBackgroundColor > 0);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::BACKGROUND_COLOR: {
    settings->setBackgroundColor(messageBegin);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::TIME_COLOR: {
    settings->setTimeColor(messageBegin);
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::UTC_TIME_OFFSET: {
    int offset = atoi(messageBegin);
    settings->setUtcHourOffset(offset);
    if (adjustSummertime(&rtc, &timeClient, offset, wifiConnected) != true) {
      Serial.println("Failed to adjust time");
      error = Error::SUMMERTIME_ERROR;
    }
    TIME time;
    if (updateTime(&time) == false) {
      break;
    }
    notifyClients();
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::WORDCONFIG: {
    settings->setWordConfig(messageBegin);
    continueWordConfig();
    break;
  }
  case MessageId::FINISHED_WORDCONFIG: {
    settings->saveWordConfig();
    notifyClients();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::CLEAR_WORDCONFIG: {
    settings->clearWordConfig();
    break;
  }
  case MessageId::CONTINUE_SEND_WORDCONFIG: {
    uint8_t index = atoi(messageBegin);
    sendWordConfigToWeb(index);
    break;
  }
  case MessageId::GET_VALUES: {
    sendSettingsToWeb();
    sendStarttime();
    sendWordConfigToWeb(0);
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::GET_TIME: {
    updateTimeOnWeb();
    sendPreviewToWeb(0);
    break;
  }
  case MessageId::CONTINUE_SEND_PREVIEW: {
    uint16_t startIdx = atoi(messageBegin);
    if (startIdx < NUMPIXELS) {
      sendPreviewToWeb(startIdx);
    }
    break;
  }
  case MessageId::UNKNOWN: {
    break;
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

LANGUAGE getLanguageKey(const char *name) {
  map_lang_string_to_enum return MAX_LANGUAGES;
}

void loadWordConfig() {
  File file = LittleFS.open("/words.json", "r");
  uint8_t i{0};
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
        strcpy(words[i].properties[lang].name,
               langProperties["name"].as<const char *>());
        words[i].properties[lang].startPixel =
            langProperties["startPixel"].as<uint16_t>();
        words[i].properties[lang].length =
            langProperties["length"].as<uint8_t>();
      }
    }
    ++i;
  } while (file.findUntil(",", "]"));

  file.close();
  Serial.println("Loaded words");

#if DEBUG
  serializeJsonPretty(words, Serial);
#endif
}

void setup() {
  /* Hardware setup */
  Serial.begin(9600);
  Wire.begin(D2, D1);

  /* Setup ledsrip */
  pixels.begin();
  memset(oldColor, 0, sizeof(oldColor));
  memset(newColor, 0, sizeof(newColor));

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
  pixels.show();
  Serial.print("Settings size: ");
  Serial.println(sizeof(Settings));
  Serial.print("words size: ");
  Serial.println(sizeof(words));
  Serial.print("Timeprocessor size: ");
  Serial.println(sizeof(TimeProcessor));
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

bool setStartTime = false;

void loop() {

  TIME time;
  time.valid = false;

  /* Starttimer */
  unsigned long start = millis();

  /* Display all red leds in red in case of an error. */
  if (error != Error::OK) {
    for (u_int8_t i = 0; i < NUMPIXELS; ++i) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
    pixels.show();
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

  /* Check if the summertime needs to be adjusted and if so, do so */
  if (millis() - lastDaylightCheck > checkDaylightTime) {
    if (adjustSummertime(&rtc, &timeClient, settings->getUtcHourOffset(),
                         wifiConnected) != true) {
      error = Error::SUMMERTIME_ERROR;
      return;
    }
    lastDaylightCheck = millis();
  }

  /* Get the current time */
  if (millis() - lastTimeUpdate > updateTimeInterval) {
    if (updateTime(&time) == false) {
      return;
    }
    setLeds();
    if (!setStartTime) {
      snprintf(startTime, sizeof(startTime), "%02d:%02d:%02d %02d.%02d.%04d",
               time.hour, time.minute, time.seconds, time.day, time.month,
               time.year);
      setStartTime = true;
    }
    lastTimeUpdate = millis();
  }

  /* Update the time on the rtc from the ntp time */
  if (millis() - lastRtcSync > syncRtc && rtc.found == true) {
    if (updateRtcTime(&rtc, &time, wifiConnected) == false) {
      error = Error::UPDATE_RTC_TIME_ERROR;
      return;
    }
    lastRtcSync = millis();
  }

  interpolateLeds();
  pixels.show();

  unsigned long end = millis();

  if (cycleTimeMs > end - start) {
    delay(cycleTimeMs - end + start);
  }
}
