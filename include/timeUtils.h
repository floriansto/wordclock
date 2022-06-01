#ifndef __TIMEUTILS_H__
#define __TIMEUTILS_H__

bool updateRtcTime(RTC *rtc, TIME *time, bool wifiConnected);
TIME getTimeNtp(NTPClient *timeClient);
TIME getTimeRtc(RTC_DS3231 *rtc);
TIME getTime(RTC *rtc, NTPClient *timeClient, bool wifiConnected);
bool summertime_EU(TIME time, s8_t tzHours);
bool adjustSummertime(RTC *rtc, NTPClient *timeClient, s8_t utcHourOffsets,
                      bool wifiConnected);

#endif
