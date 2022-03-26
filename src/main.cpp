#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <math.h>
#include "../includes_gen/index.h"
#include "../includes_gen/css.h"
#include "../includes/main.h"

#define PIN D6
#define COL_PIXELS 5
#define ROW_PIXELS 14
#define NUMPIXELS (COL_PIXELS * ROW_PIXELS)
#define INITIAL_BRIGHTNESS 10

const double max_current_ma = 400.0;
const double allowed_current_per_color_ma = (max_current_ma / (double)NUMPIXELS / 4.0);
const double max_current_per_color_ma = 11.0;
const double scale = allowed_current_per_color_ma / max_current_per_color_ma;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(ROW_PIXELS, COL_PIXELS, PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                                   NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);

u_int8_t brightness = INITIAL_BRIGHTNESS;

u_int8_t riseHour(u_int8_t hour)
{
  return hour == 12 ? 1 : ++hour;
}

ClockStr getStateFromNum(u_int8_t num)
{
  get_enum_from_num(num);
}

String getString(ClockStr state)
{
  get_str_from_enum(state);
}

String evalTime(time_t epochTime)
{
  ClockStr stack[10];
  struct tm *ptm = gmtime((time_t *)&epochTime);
  u_int8_t hour = fmod(ptm->tm_hour, 12);
  u_int16_t seconds = ptm->tm_sec + ptm->tm_min * 60;
  u_int8_t i = 0;
  bool useQuaterPast = false;
  bool useThreeQuater = true;

  if (hour == 0)
  {
    hour = 12;
  }
  
  stack[i++] = ClockStr::It;
  stack[i++] = ClockStr::Is;
  // Five after
  if (seconds > 150 && seconds < 450)
  {
    stack[i++] = ClockStr::Five;
    stack[i++] = ClockStr::After;
  }
  // Ten after
  else if (seconds >= 450 && seconds < 750)
  {
    stack[i++] = ClockStr::Ten;
    stack[i++] = ClockStr::After;
  }
  // Fiveteen after
  else if (seconds >= 750 && seconds < 1050)
  {
    stack[i++] = ClockStr::Quater;
    if (useQuaterPast)
      stack[i++] = ClockStr::After;
    else
      hour = riseHour(hour);
  }
  else if (seconds >= 1050)
  {
    hour = riseHour(hour);
    // Ten before half
    if (seconds >= 1050 && seconds < 1350)
    {
      stack[i++] = ClockStr::Ten;
      stack[i++] = ClockStr::Before;
      stack[i++] = ClockStr::Half;
    }
    // Five before half
    else if (seconds >= 1350 && seconds < 1650)
    {
      stack[i++] = ClockStr::Five;
      stack[i++] = ClockStr::Before;
      stack[i++] = ClockStr::Half;
    }
    // Half
    else if (seconds >= 1650 && seconds < 1950)
    {
      stack[i++] = ClockStr::Five;
      stack[i++] = ClockStr::After;
      stack[i++] = ClockStr::Half;
    }
    // Five after half
    else if (seconds >= 1950 && seconds < 2250)
    {
      stack[i++] = ClockStr::Five;
      stack[i++] = ClockStr::After;
      stack[i++] = ClockStr::Half;
    }
    // Ten after half
    else if (seconds >= 2250 && seconds < 2550)
    {
      stack[i++] = ClockStr::Ten;
      stack[i++] = ClockStr::After;
      stack[i++] = ClockStr::Half;
    }
    else if (seconds >= 2550 && seconds < 2850)
    {
      // Quater before
      if (useThreeQuater)
      {
        stack[i++] = ClockStr::ThreeQuater;
      }
      else
      {
        stack[i++] = ClockStr::Quater;
        stack[i++] = ClockStr::Before;
      }
    }
    // Ten before
    else if (seconds >= 2850 && seconds < 3150)
    {
      stack[i++] = ClockStr::Ten;
      stack[i++] = ClockStr::Before;
    }
    // Five before
    else if (seconds >= 3150 && seconds < 3450)
    {
      stack[i++] = ClockStr::Five;
      stack[i++] = ClockStr::Before;
    }
  }

  stack[i++] = getStateFromNum(hour);
  if (seconds <= 150 || seconds >= 3450)
  {
    if (stack[i - 1] == ClockStr::One)
    {
      stack[i - 1] = ClockStr::OneEven;
    }
    stack[i++] = ClockStr::Clock;
  }
  
  String time = "";
  for (u_int8_t j = 0; j < i; ++j)
  {
    time += getString(stack[j]) + " ";
  }
  
  return time;
}

void handleOnConnect()
{
  brightness = INITIAL_BRIGHTNESS;
  server.send(200, "text/html", index_html);
}

void handleCss()
{
  server.send(200, "text/css", style_css);
}

void handleBrightness()
{
  String state = server.arg("LEDBrightness");
  Serial.println(state);
  if (state == "up")
    brightness = brightness > 95 ? 100 : brightness + 5;
  else
    brightness = brightness < 5 ? 0 : brightness - 5;
  server.send(200, "text/plain", String(brightness));
  matrix.setBrightness(brightness * scale);
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not found");
}

void handleGetBrightness()
{
  server.send(200, "text/plain", String(brightness));
}

void handleGetTime()
{
  server.send(200, "text/plain", String(timeClient.getFormattedTime()));
}

void handleGetWordTime()
{
  server.send(200, "text/plain", evalTime(timeClient.getEpochTime()));
}

void setup()
{
  Serial.begin(9600);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness * scale);
  Serial.println(scale);
  matrix.setTextColor(matrix.Color(255, 0, 0));

  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(60);
  if (wifiManager.autoConnect("Wordclock"))
  {
    Serial.println("Connected to wifi :)");
  }
  else
  {
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

void loop()
{
  u_int16_t color;

  timeClient.update();
  server.handleClient();

  if (millis() - lastRun > evalTimeEvery)
  {
    lastRun = millis();
    Serial.println(timeClient.getFormattedTime());
    Serial.println(evalTime(timeClient.getEpochTime()));
  }

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
