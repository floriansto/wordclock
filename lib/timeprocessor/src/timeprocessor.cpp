#include <Arduino.h>
#include <WString.h>
#include <math.h>
#include <time.h>
#include <timeprocessor.h>
#include <timestack.h>

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

bool TimeProcessor::setTime5(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Five, false});
  ret &= stack->push(TIMESTACK{ClockStr::After, getDialect()});
  return ret;
}

bool TimeProcessor::setTime10(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Ten, false});
  ret &= stack->push(TIMESTACK{ClockStr::After, getDialect()});
  return ret;
}

bool TimeProcessor::setTime15(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Quater, getDialect()});
  if (getQuaterPast())
    ret &= stack->push(TIMESTACK{ClockStr::After, getDialect()});
  return ret;
}

bool TimeProcessor::setTime20(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Ten, false});
  ret &= stack->push(TIMESTACK{ClockStr::Before, getDialect()});
  ret &= stack->push(TIMESTACK{ClockStr::Half, false});
  return ret;
}

bool TimeProcessor::setTime25(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Five, false});
  ret &= stack->push(TIMESTACK{ClockStr::Before, getDialect()});
  ret &= stack->push(TIMESTACK{ClockStr::Half, false});
  return ret;
}

bool TimeProcessor::setTime30(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Half, getDialect()});
  return ret;
}

bool TimeProcessor::setTime35(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Five, false});
  ret &= stack->push(TIMESTACK{ClockStr::After, getDialect()});
  ret &= stack->push(TIMESTACK{ClockStr::Half, false});
  return ret;
}

bool TimeProcessor::setTime40(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Ten, false});
  ret &= stack->push(TIMESTACK{ClockStr::After, getDialect()});
  ret &= stack->push(TIMESTACK{ClockStr::Half, false});
  return ret;
}

bool TimeProcessor::setTime45(Timestack *stack) {
  bool ret = true;
  if (getThreeQuater()) {
    ret = stack->push(TIMESTACK{ClockStr::ThreeQuater, getDialect()});
  } else {
    ret = stack->push(TIMESTACK{ClockStr::Quater, getDialect()});
    ret &= stack->push(TIMESTACK{ClockStr::Before, getDialect()});
  }
  return ret;
}

bool TimeProcessor::setTime50(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Ten, false});
  ret &= stack->push(TIMESTACK{ClockStr::Before, getDialect()});
  return ret;
}

bool TimeProcessor::setTime55(Timestack *stack) {
  bool ret = true;
  ret = stack->push(TIMESTACK{ClockStr::Five, false});
  ret &= stack->push(TIMESTACK{ClockStr::Before, getDialect()});
  return ret;
}

bool TimeProcessor::getTimeStack(Timestack *stack, time_t epochTime) {
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int hour = fmod(ptm->tm_hour, 12);
  int seconds = ptm->tm_sec + ptm->tm_min * 60;
  bool ret;
  String time = "";
  TIMESTACK elem;

  if (hour == 0) {
    hour = 12;
  }

  if (seconds > getHighBorder(15 * 60)) {
    hour = riseHour(hour);
  }

  if (checkInterval(seconds, 15 * 60) && !getQuaterPast()) {
    hour = riseHour(hour);
  }

  ret = stack->push(TIMESTACK{ClockStr::It, getDialect()});
  ret &= stack->push(TIMESTACK{ClockStr::Is, getDialect()});

  for (int j = 5; j < 60; j += 5) {
    if (checkInterval(seconds, j * 60)) {
      call_times_fcn(j);
      break;
    }
  }

  ret &= stack->push(TIMESTACK{getStateFromNum(hour), getDialect()});
  if (checkInterval(seconds, 0)) {
    ret &= stack->get(&elem, stack->getSize() - 1);
    if (!ret && elem.state == ClockStr::One) {
      ret &= stack->push(TIMESTACK{ClockStr::OneEven, getDialect()});
    }
    if (!getDialect()) {
      ret &= stack->push(TIMESTACK{ClockStr::Clock, getDialect()});
    }
  }

  return ret;
}

String TimeProcessor::evalTime(time_t epochTime) {
  Timestack stack;
  String time;
  TIMESTACK elem;
  bool ret;

  ret = getTimeStack(&stack, epochTime);

  for (int j = 0; j < stack.getSize(); ++j) {
    ret = stack.get(&elem, j);
    if (ret) {
      time += getString(&elem) + " ";
    }
  }
  return time;
}
