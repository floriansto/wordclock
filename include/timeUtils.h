#ifndef __TIMEUTILS_H__
#define __TIMEUTILS_H__

bool updateRtcTime(RTC *rtc, NTPClient *ntp);
TIME getTimeNtp(NTPClient *timeClient);
TIME getTimeRtc(RTC_DS3231 *rtc);
TIME getTime(RTC *rtc, NTPClient *ntp);
bool summertime_EU(TIME time, s8_t tzHours);
bool adjustSummertime(RTC *rtc, NTPClient *ntp, s8_t utcHourOffsets,
                      bool isSummertime);

#endif
