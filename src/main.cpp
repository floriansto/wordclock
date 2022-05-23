#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <RTClib.h>
#include <WiFiManager.h>
#include <math.h>
#include <timeprocessor.h>

#include "../include/main.h"
#include "../include/settings.h"
#include "../includes_gen/css.h"
#include "../includes_gen/index.h"

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
    ROW_PIXELS, COL_PIXELS, PIN,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

WiFiManager wifiManager;
WiFiUDP ntpUDP;
RTC_DS3231 rtc;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);
TimeProcessor *timeProcessor =
    new TimeProcessor(useDialect, useQuaterPast, useThreeQuater, offsetLowSecs,
                      offsetHighSecs, NUMPIXELS);

bool rtcFound = true;
bool rtcValid = true;
bool error = false;
bool wifiConnected = false;
u_int16_t active_leds_loop = 0;
u_int8_t brightness = INITIAL_BRIGHTNESS;

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
  double current_per_color = max_current_ma / (double) active_leds / 3.0;
  return current_per_color / max_current_per_color_ma;
}

void handleOnConnect() {
  server.send(200, "text/html", index_html);
}

void handleCss() { server.send(200, "text/css", style_css); }

void handleBrightness() {
  String state = server.arg("LEDBrightness");
  //Serial.println(state);
  brightness = state.toInt();
  server.send(200, "text/plain", state);
  //matrix.setBrightness(brightness * calc_scale(active_leds_loop));
}

void handleDialect() {
  String state = server.arg("switchDialect");
  Serial.println(state);
  if (state == "true") {
    useDialect = true;
  } else {
    useDialect = false;
  }
  timeProcessor->setDialect(useDialect);
  server.send(200, "text/plain", state);
}

void handleNotFound() { server.send(404, "text/plain", "Not found"); }

void handleGetBrightness() {
  server.send(200, "text/plain", String(brightness));
}

void handleGetTime() {
  TIME time = getTime();
  char timeStr[12];
  sprintf(timeStr, "%02d:%02d:%02d", time.hour, time.minute, time.seconds);
  server.send(200, "text/plain", timeStr);
}

void handleGetWordTime() {
  char wordTime[NUMPIXELS];
  char error[] = "Error getting the time";
  if (timeProcessor->getWordTime(wordTime)) {
    server.send(200, "text/plain", wordTime);
  } else {
    server.send(200, "text/plain", error);
  }
}

void handleGetDialect() {
  server.send(200, "text/plain", String((int)useDialect));
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
  server.stop();
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

void setup() {
  TIME time;

  Serial.begin(9600);
  delay(1000);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness * calc_scale(NUMPIXELS));
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalBlocking(false);

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

  server.on("/", handleOnConnect);
  server.on("/style.css", handleCss);
  server.on("/setBrightness", handleBrightness);
  server.on("/setDialect", handleDialect);
  server.on("/readBrightness", handleGetBrightness);
  server.on("/readTime", handleGetTime);
  server.on("/readWordTime", handleGetWordTime);
  server.on("/readDialect", handleGetDialect);
  server.onNotFound(handleNotFound);

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

  wifiManager.process();
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

  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
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

  matrix.setBrightness(brightness * calc_scale(active_leds_loop));
  matrix.show();

  delay(10);
}
