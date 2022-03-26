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
TimeProcessor *wordProcessor = new TimeProcessor(
    useDialect, useQuaterPast, useThreeQuater, offsetLowSecs, offsetHighSecs);

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

void handleNotFound() { server.send(404, "text/plain", "Not found"); }

void handleGetBrightness() {
  server.send(200, "text/plain", String(brightness));
}

void handleGetTime() {
  server.send(200, "text/plain", String(timeClient.getFormattedTime()));
}

void handleGetWordTime() {
  server.send(200, "text/plain",
              wordProcessor->evalTime(timeClient.getEpochTime()));
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
  server.on("/readBrightness", handleGetBrightness);
  server.on("/readTime", handleGetTime);
  server.on("/readWordTime", handleGetWordTime);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  timeClient.begin();
  Serial.println("NTP client started");

  timeClient.setTimeOffset(3600);
}

unsigned long lastRun = 0;
u_int16_t evalTimeEvery = 60000;

void loop() {
  u_int16_t color;

  timeClient.update();
  server.handleClient();

  if (millis() - lastRun > evalTimeEvery) {
    lastRun = millis();
    Serial.println(timeClient.getFormattedTime());
    Serial.println(wordProcessor->evalTime(timeClient.getEpochTime()));
  }

  color = matrix.Color(0, 255, 0);
  error |= (WiFi.status() == WL_CONNECT_FAILED ||
            WiFi.status() == WL_CONNECTION_LOST);
  if (error) {
    color = matrix.Color(255, 0, 0);
  }

  matrix.fillScreen(0);
  matrix.fillCircle(7, 2, 2, color);
  matrix.show();
  delay(10);
}
