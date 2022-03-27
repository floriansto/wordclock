#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiManager.h>
#include <math.h>
#include <timeprocessor.h>

#include "../include/settings.h"
#include "../includes_gen/css.h"
#include "../includes_gen/index.h"

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
    ROW_PIXELS, COL_PIXELS, PIN,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800);

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);
TimeProcessor *timeProcessor =
    new TimeProcessor(useDialect, useQuaterPast, useThreeQuater, offsetLowSecs,
                      offsetHighSecs, NUMPIXELS);

bool error = false;

u_int8_t brightness = INITIAL_BRIGHTNESS;

void handleOnConnect() {
  brightness = INITIAL_BRIGHTNESS;
  server.send(200, "text/html", index_html);
}

void handleCss() { server.send(200, "text/css", style_css); }

void handleBrightness() {
  String state = server.arg("LEDBrightness");
  Serial.println(state);
  if (state == "up")
    brightness = brightness > 95 ? 100 : brightness + 5;
  else
    brightness = brightness < 5 ? 0 : brightness - 5;
  server.send(200, "text/plain", String(brightness));
  matrix.setBrightness(brightness * scale);
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
  server.send(200, "text/plain", String(timeClient.getFormattedTime()));
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
boolean summertime_EU(time_t epochTime, s8_t tzHours) {
  struct tm *ptm = gmtime((time_t *)&epochTime);
  u_int16_t year = ptm->tm_year + 1900;
  u_int8_t month = ptm->tm_mon + 1;
  u_int8_t day = ptm->tm_mday;
  u_int8_t hour = ptm->tm_hour;

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

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness * scale);
  Serial.println(scale);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(60);
  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
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

  server.begin();
  Serial.println("HTTP server started");

  timeClient.begin();
  Serial.println("NTP client started");

  timeClient.setTimeOffset(hourOffsetFromUTC * 3600);
}

unsigned long lastRun = 0;
u_int16_t evalTimeEvery = 60000;
unsigned long lastDaylightCheck = 0;
u_int32_t checkDaylightTime = 3600000;

void loop() {
  u_int16_t color;
  char wordTime[NUMPIXELS];

  timeClient.update();
  server.handleClient();

  color = matrix.Color(0, 255, 0);
  error |= (WiFi.status() == WL_CONNECT_FAILED ||
            WiFi.status() == WL_CONNECTION_LOST);
  if (error) {
    color = matrix.Color(255, 0, 0);
  }

  matrix.fillScreen(0);
  matrix.fillCircle(7, 2, 2, color);
  matrix.show();

  if (!timeProcessor->update(timeClient.getEpochTime())) {
    Serial.println("Error occured");
    error = true;
    return;
  }

  if (millis() - lastDaylightCheck > checkDaylightTime) {
    if (summertime_EU(timeClient.getEpochTime(), hourOffsetFromUTC)) {
      timeClient.setTimeOffset((hourOffsetFromUTC + 1) * 3600);
    }
  }

  if (millis() - lastRun > evalTimeEvery) {
    lastRun = millis();
    Serial.println(timeClient.getFormattedTime());
    if (timeProcessor->getWordTime(wordTime)) {
      Serial.println(wordTime);
    }
  }

  delay(10);
}
