#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <math.h>

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
ESP8266WebServer server(80);

int delayval = 100;
int brightness = INITIAL_BRIGHTNESS;

void handle_OnConnect() {
  brightness = INITIAL_BRIGHTNESS;
  Serial.println("Brightness: 100%");
  server.send(200, "text/html", SendHTML());
}

void handle_brightnessup() {
  brightness = brightness > 95 ? 100 : brightness + 5;
  server.send(200, "text/html", SendHTML());
}

void handle_brightnessdown() {
  brightness = brightness < 5 ? 0 : brightness - 5;
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setConfigPortalTimeout(60);
  if (wifiManager.autoConnect("Wordclock")) {
    Serial.println("Connected to wifi :)");
  } else {
    Serial.println("Configportal at 192.168.4.1 running");
  }

  server.on("/", handle_OnConnect);
  server.on("/brighter", handle_brightnessup);
  server.on("/darker", handle_brightnessdown);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  double scale = 1.0;
  int color;

  wifiManager.process();
  server.handleClient();

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
  delay(delayval);
}

String SendHTML() {
  char br[33];
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Wordclock</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;text_align: center} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: inline-block;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Wordclock Web Server</h1>\n";

  ptr +="<p>LED Brightness Up</p><a class=\"button button-on\" href=\"/brighter\">BRIGHTER</a>\n";
  ptr +="<p>LED Brightness Down</p><a class=\"button button-off\" href=\"/darker\">DARKER</a>\n";
  sprintf(br, "<p>Current Brightness<br>%d</p>", brightness);
  ptr += br;

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
