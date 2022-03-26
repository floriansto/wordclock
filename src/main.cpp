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
#define WORDSTACK_SIZE 7

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
u_int16_t offsetLeftSecs = 0;
u_int16_t offsetRightSecs = -offsetLeftSecs + 300;
bool error = false;


u_int8_t brightness = INITIAL_BRIGHTNESS;

u_int8_t riseHour(u_int8_t hour)
{
  return hour == 12 ? 1 : ++hour;
}

ClockStr getStateFromNum(u_int8_t num)
{
  get_enum_from_num(num);
}

String getString(TIMESTACK *element)
{
  if (element->useDialect)
  {
    get_str_dialect_from_enum(element->state);
  }
  else
  {
    get_str_from_enum(element->state);
  }
}

bool writeToStack(TIMESTACK* stack, ClockStr state, bool useDialect, u_int8_t *stack_size)
{
  if (*stack_size == WORDSTACK_SIZE)
  {
    error = true;
    return false;
  }
  stack[*stack_size].state = state;
  stack[*stack_size].useDialect = useDialect;
  ++(*stack_size);
  return true;
}

u_int16_t getHighBorder(u_int16_t time)
{
  return time > 3600 - offsetRightSecs ? time - 3600 - offsetRightSecs : time + offsetRightSecs;
}

u_int16_t getLowBorder(u_int16_t time)
{
  return time < offsetLeftSecs ? 3600 + time - offsetLeftSecs : time - offsetLeftSecs;
}


bool checkInterval(u_int16_t seconds, u_int16_t secs_check)
{
  return seconds >= getLowBorder(secs_check) &&
         seconds < getHighBorder(secs_check);
}

u_int8_t getTimeStack(TIMESTACK *stack, time_t epochTime)
{
  struct tm *ptm = gmtime((time_t *)&epochTime);
  u_int8_t hour = fmod(ptm->tm_hour, 12);
  u_int16_t seconds = ptm->tm_sec + ptm->tm_min * 60;
  u_int8_t i = 0;
  String time = "";

  bool useQuaterPast = true;
  bool useThreeQuater = true;
  bool useDialect = true;

  if (hour == 0)
  {
    hour = 12;
  }
  
  writeToStack(stack, ClockStr::It, useDialect, &i);
  writeToStack(stack, ClockStr::Is, useDialect, &i);
  // Five after
  if (checkInterval(seconds, 5*60))
  {
    writeToStack(stack, ClockStr::Five, false, &i);
    writeToStack(stack, ClockStr::After, useDialect, &i);
  }
  // Ten after
  else if (checkInterval(seconds, 10*60))
  {
    writeToStack(stack, ClockStr::Ten, false, &i);
    writeToStack(stack, ClockStr::After, useDialect, &i);
  }
  // Fiveteen after
  else if (checkInterval(seconds, 15*60))
  {
    writeToStack(stack, ClockStr::Quater, useDialect, &i);
    if (useQuaterPast)
      writeToStack(stack, ClockStr::After, useDialect, &i);
    else
      hour = riseHour(hour);
  }
  else if (seconds >= getHighBorder(15 * 60))
  {
    hour = riseHour(hour);
    // Ten before half
    if (checkInterval(seconds, 20*60))
    {
      writeToStack(stack, ClockStr::Ten, false, &i);
      writeToStack(stack, ClockStr::Before, useDialect, &i);
      writeToStack(stack, ClockStr::Half, false, &i);
    }
    // Five before half
    else if (checkInterval(seconds, 25*60))
    {
      writeToStack(stack, ClockStr::Five, false, &i);
      writeToStack(stack, ClockStr::Before, useDialect, &i);
      writeToStack(stack, ClockStr::Half, false, &i);
    }
    // Half
    else if (checkInterval(seconds, 30*60))
    {
      writeToStack(stack, ClockStr::Half, useDialect, &i);
    }
    // Five after half
    else if (checkInterval(seconds, 35*60))
    {
      writeToStack(stack, ClockStr::Five, false, &i);
      writeToStack(stack, ClockStr::After, useDialect, &i);
      writeToStack(stack, ClockStr::Half, false, &i);
    }
    // Ten after half
    else if (checkInterval(seconds, 40*60))
    {
      writeToStack(stack, ClockStr::Ten, false, &i);
      writeToStack(stack, ClockStr::After, useDialect, &i);
      writeToStack(stack, ClockStr::Half, false, &i);
    }
    else if (checkInterval(seconds, 45*60))
    {
      // Quater before
      if (useThreeQuater)
      {
        writeToStack(stack, ClockStr::ThreeQuater, useDialect, &i);
      }
      else
      {
        writeToStack(stack, ClockStr::Quater, useDialect, &i);
        writeToStack(stack, ClockStr::Before, useDialect, &i);
      }
    }
    // Ten before
    else if (checkInterval(seconds, 50*60))
    {
      writeToStack(stack, ClockStr::Ten, false, &i);
      writeToStack(stack, ClockStr::Before, useDialect, &i);
    }
    // Five before
    else if (checkInterval(seconds, 55*60))
    {
      writeToStack(stack, ClockStr::Five, false, &i);
      writeToStack(stack, ClockStr::Before, useDialect, &i);
    }
  }

  writeToStack(stack, getStateFromNum(hour), useDialect, &i);
  if (checkInterval(seconds, 0))
  {
    if (stack[i - 1].state == ClockStr::One)
    {
      writeToStack(stack, ClockStr::OneEven, useDialect, &i);
    }
    if (!useDialect)
    {
      writeToStack(stack, ClockStr::Clock, useDialect, &i);
    }
  }
  
  return i;
}

String evalTime(time_t epochTime)
{
  TIMESTACK stack[WORDSTACK_SIZE];
  String time;

  u_int8_t i = getTimeStack(stack, epochTime);

  for (u_int8_t j = 0; j < i; ++j)
  {
    time += getString(&stack[j]) + " ";
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

  color = matrix.Color(0, 255, 0);
  error |= (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST);
  if (error)
  {
    color = matrix.Color(255, 0, 0);
  }

  matrix.fillScreen(0);
  matrix.fillCircle(7, 2, 2, color);
  matrix.show();
  delay(10);
}
