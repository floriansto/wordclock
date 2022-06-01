#include <Arduino.h>
#include <NTPClient.h>
#include <RTClib.h>

#include "../include/main.h"
#include "../include/timeUtils.h"

bool updateRtcTime(RTC *rtc, TIME *time, bool wifiConnected) {
  if (wifiConnected == true) {
    Serial.println("Missing wifi connection, cannot adjust NTP time");
    rtc->valid = false;
    return false;
  }

  if (rtc->found == false) {
    Serial.println("RTC missing, cannot update time on the rtc");
    rtc->valid = false;
    return false;
  }

  Serial.println("Adjust RTC time from NTP time");
  rtc->rtc.adjust(DateTime(time->year, time->month, time->day, time->hour,
                           time->minute, time->seconds));
  rtc->valid = true;
  return true;
}

TIME getTimeRtc(RTC_DS3231 *rtc) {
  TIME time;
  DateTime now{rtc->now()};
  time.hour = now.hour();
  time.minute = now.minute();
  time.seconds = now.second();
  time.year = now.year();
  time.month = now.month();
  time.day = now.day();
  time.valid = true;
  return time;
}

TIME getTimeNtp(NTPClient *timeClient) {
  TIME time;
  timeClient->update();
  time.hour = timeClient->getHours();
  time.minute = timeClient->getMinutes();
  time.seconds = timeClient->getSeconds();
  time_t epochTime = timeClient->getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  time.year = ptm->tm_year + 1900;
  time.month = ptm->tm_mon + 1;
  time.day = ptm->tm_mday;
  time.valid = true;
  return time;
}

TIME getTime(RTC *rtc, NTPClient *timeClient, bool wifiConnected) {
  TIME time;
  memset(&time, 0, sizeof(time));
  if (wifiConnected == false && rtc->valid == true) {
    return getTimeRtc(&(rtc->rtc));
  } else if (wifiConnected == true) {
    return getTimeNtp(timeClient);
  }
  time.valid = false;
  return time;
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
bool summertime_EU(TIME time, s8_t tzHours) {
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

bool adjustSummertime(RTC *rtc, NTPClient *timeClient, s8_t utcHourOffsets,
                      bool wifiConnected) {
  Serial.println("Set summertime offset");
  TIME time = getTime(rtc, timeClient, wifiConnected);
  if (time.valid == false) {
    return false;
  }
  if (summertime_EU(time, utcHourOffsets) == true) {
    timeClient->setTimeOffset((utcHourOffsets + 1) * 3600);
  } else {
    timeClient->setTimeOffset(utcHourOffsets * 3600);
  }
  timeClient->update();

  if (updateRtcTime(rtc, &time, wifiConnected) == false) {
    return false;
  }
  return true;
}
