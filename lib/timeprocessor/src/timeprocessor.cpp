#include <WString.h>
#include <math.h>
#include <time.h>
#include <timeprocessor.h>

int riseHour(int hour) { return hour == 12 ? 1 : ++hour; }

ClockStr getStateFromNum(int num) { get_enum_from_num(num); }

String getString(TIMESTACK *element) {
  if (element->useDialect) {
    get_str_dialect_from_enum(element->state);
  } else {
    get_str_from_enum(element->state);
  }
}

bool writeToStack(TIMESTACK *stack, ClockStr state, bool useDialect,
                  int *stack_size) {
  if (*stack_size == WORDSTACK_SIZE) {
    return false;
  }
  stack[*stack_size].state = state;
  stack[*stack_size].useDialect = useDialect;
  ++(*stack_size);
  return true;
}

int TimeProcessor::getOffsetLowSecs() { return m_offsetLowSecs; }
int TimeProcessor::getOffsetHighSecs() { return m_offsetHighSecs; }
bool TimeProcessor::getDialect() { return m_useDialect; }
bool TimeProcessor::getQuaterPast() { return m_useQuaterPast; }
bool TimeProcessor::getThreeQuater() { return m_useThreeQuater; }

void TimeProcessor::setQuaterPast(bool useQuaterPast) {
  m_useQuaterPast = useQuaterPast;
}

void TimeProcessor::setThreeQuater(bool useThreeQuater) {
  m_useThreeQuater = useThreeQuater;
}

void TimeProcessor::setOffsetHighSecs(int offsetHighSecs) {
  m_offsetHighSecs = offsetHighSecs;
}

void TimeProcessor::setOffsetLowSecs(int offsetLowSecs) {
  m_offsetLowSecs = offsetLowSecs;
}

void TimeProcessor::setDialect(bool dialect) { m_useDialect = dialect; }

int TimeProcessor::getHighBorder(int time) {
  int offsetHighSecs = getOffsetHighSecs();
  return time > 3600 - offsetHighSecs ? time - 3600 - offsetHighSecs
                                      : time + offsetHighSecs;
}

int TimeProcessor::getLowBorder(int time) {
  int offsetLowSecs = getOffsetLowSecs();
  return time < offsetLowSecs ? 3600 + time - offsetLowSecs
                              : time - offsetLowSecs;
}

bool TimeProcessor::checkInterval(int seconds, int target) {
  return seconds >= getLowBorder(target) && seconds < getHighBorder(target);
}

bool TimeProcessor::setTime5(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Five, false, i);
  ret &= writeToStack(stack, ClockStr::After, getDialect(), i);
  return ret;
}

bool TimeProcessor::setTime10(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Ten, false, i);
  ret &= writeToStack(stack, ClockStr::After, getDialect(), i);
  return ret;
}

bool TimeProcessor::setTime15(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Quater, getDialect(), i);
  if (getQuaterPast())
    ret &= writeToStack(stack, ClockStr::After, getDialect(), i);
  return ret;
}

bool TimeProcessor::setTime20(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Ten, false, i);
  ret &= writeToStack(stack, ClockStr::Before, getDialect(), i);
  ret &= writeToStack(stack, ClockStr::Half, false, i);
  return ret;
}

bool TimeProcessor::setTime25(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Five, false, i);
  ret &= writeToStack(stack, ClockStr::Before, getDialect(), i);
  ret &= writeToStack(stack, ClockStr::Half, false, i);
  return ret;
}

bool TimeProcessor::setTime30(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Half, getDialect(), i);
  return ret;
}

bool TimeProcessor::setTime35(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Five, false, i);
  ret &= writeToStack(stack, ClockStr::After, getDialect(), i);
  ret &= writeToStack(stack, ClockStr::Half, false, i);
  return ret;
}

bool TimeProcessor::setTime40(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Ten, false, i);
  ret &= writeToStack(stack, ClockStr::After, getDialect(), i);
  ret &= writeToStack(stack, ClockStr::Half, false, i);
  return ret;
}

bool TimeProcessor::setTime45(TIMESTACK *stack, int *i) {
  bool ret = true;
  if (getThreeQuater()) {
    ret = writeToStack(stack, ClockStr::ThreeQuater, getDialect(), i);
  } else {
    ret = writeToStack(stack, ClockStr::Quater, getDialect(), i);
    ret &= writeToStack(stack, ClockStr::Before, getDialect(), i);
  }
  return ret;
}

bool TimeProcessor::setTime50(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Ten, false, i);
  ret &= writeToStack(stack, ClockStr::Before, getDialect(), i);
  return ret;
}

bool TimeProcessor::setTime55(TIMESTACK *stack, int *i) {
  bool ret = true;
  ret = writeToStack(stack, ClockStr::Five, false, i);
  ret &= writeToStack(stack, ClockStr::Before, getDialect(), i);
  return ret;
}

int TimeProcessor::getTimeStack(TIMESTACK *stack, time_t epochTime) {
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int hour = fmod(ptm->tm_hour, 12);
  int seconds = ptm->tm_sec + ptm->tm_min * 60;
  int i = 0;
  bool ret;
  String time = "";

  bool useDialect = true;

  if (hour == 0) {
    hour = 12;
  }

  if (seconds > getHighBorder(15 * 60)) {
    hour = riseHour(hour);
  }

  if (checkInterval(seconds, 15 * 60) && !getQuaterPast()) {
    hour = riseHour(hour);
  }

  ret = writeToStack(stack, ClockStr::It, useDialect, &i);
  ret &= writeToStack(stack, ClockStr::Is, useDialect, &i);

  for (int j = 5; j < 60; j += 5) {
    if (checkInterval(seconds, j * 60)) {
      call_times_fcn(j);
      break;
    }
  }

  ret &= writeToStack(stack, getStateFromNum(hour), useDialect, &i);
  if (checkInterval(seconds, 0)) {
    if (stack[i - 1].state == ClockStr::One) {
      ret &= writeToStack(stack, ClockStr::OneEven, useDialect, &i);
    }
    if (!useDialect) {
      ret &= writeToStack(stack, ClockStr::Clock, useDialect, &i);
    }
  }

  return i;
}

String TimeProcessor::evalTime(time_t epochTime) {
  TIMESTACK stack[WORDSTACK_SIZE];
  String time;

  int i = getTimeStack(stack, epochTime);

  for (int j = 0; j < i; ++j) {
    time += getString(&stack[j]) + " ";
  }
  return time;
}
