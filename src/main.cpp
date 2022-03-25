#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <math.h>
#include "../includes_gen/index.h"
#include "../includes_gen/css.h"

#define PIN D6
#define COL_PIXELS 5
#define ROW_PIXELS 14
#define NUMPIXELS COL_PIXELS * ROW_PIXELS
#define MAX_CURRENT_MA 400.0
#define ALLOWED_CURRENT_PER_COLOR_MA (MAX_CURRENT_MA / NUMPIXELS / 4.0)
#define MAX_CURRENT_PER_COLOR_MA 11
#define INITIAL_BRIGHTNESS 10

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(ROW_PIXELS, COL_PIXELS, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);

int brightness = INITIAL_BRIGHTNESS;
String curTime = "00:00:00";

void handleOnConnect() {
  brightness = INITIAL_BRIGHTNESS;
  server.send(200, "text/html", index_html);
}

void handleCss() {
  server.send(200, "text/css", style_css);
}

void handleBrightness() {
  String state = server.arg("LEDBrightness");
  Serial.println(state);
  if (state == "up")
    brightness = brightness > 95 ? 100 : brightness + 5;
  else
    brightness = brightness < 5 ? 0 : brightness - 5;
  server.send(200, "text/plain", String(brightness));
  matrix.setBrightness(brightness);
}

void handleNotFound(){
  server.send(404, "text/plain", "Not found");
}

void handleGetBrightness(){
  server.send(200, "text/plain", String(brightness));
}

void handleGetTime() {
  server.send(200, "text/plain", String(curTime));
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);
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
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  timeClient.begin();
  Serial.println("NTP client started");
  
  timeClient.setTimeOffset(3600);
}


void loop() {
  double scale = 1.0;
  int color;

  timeClient.update();
  server.handleClient();
  
  curTime = timeClient.getFormattedTime();

  scale = (double) ALLOWED_CURRENT_PER_COLOR_MA/MAX_CURRENT_PER_COLOR_MA;
  scale = scale > 1.0 ? 1.0 : scale;
  matrix.setBrightness(brightness * scale);

  color = matrix.Color(255, 0, 0);
  if (WiFi.status() == WL_CONNECTED)
  {
    color = matrix.Color(0, 255, 0);
  }

  matrix.fillScreen(0);
  matrix.fillCircle(7, 2, 2, color);
  matrix.show();
  delay(10);
}

