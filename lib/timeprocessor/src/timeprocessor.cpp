#include <Arduino.h>
#include <WString.h>
#include <math.h>
#include <time.h>
#include <timeprocessor.h>
#include <timestack.h>

int riseHour(int hour) { return hour == 12 ? 1 : ++hour; }

ClockStr getStateFromNum(int num) { get_enum_from_num(num); }

bool getWord(TIMESTACK *elem, char *word, int maxWordLen, int *i) {
  get_word(elem->state, elem->useDialect);
  return true;
}

TimeProcessor::TimeProcessor(bool useDialect, bool useQuaterPast,
                             bool useThreeQuater, int offsetLowSecs,
                             int offsetHighSecs, int numLetters)
    : m_useDialect{useDialect}, m_useQuaterPast{useQuaterPast},
      m_useThreeQuater{useThreeQuater}, m_offsetLowSecs{offsetLowSecs},
      m_offsetHighSecs{offsetHighSecs}, m_numLetters{numLetters} {
  m_error = false;
  m_wordtime = (char *)malloc(sizeof(char) * m_numLetters);
  if (m_wordtime == nullptr) {
    m_error = true;
  }
}

TimeProcessor::~TimeProcessor() { free(m_wordtime); }

Times TimeProcessor::getTimeEnumFromSecs(int seconds) {
  get_times_enum_from_num(seconds);
  return Times::Full;
}

int TimeProcessor::getOffsetLowSecs() { return m_offsetLowSecs; }
int TimeProcessor::getOffsetHighSecs() { return m_offsetHighSecs; }
bool TimeProcessor::getDialect() { return m_useDialect; }
bool TimeProcessor::getQuaterPast() { return m_useQuaterPast; }
bool TimeProcessor::getThreeQuater() { return m_useThreeQuater; }
Timestack *TimeProcessor::getStack() { return &m_stack; }
bool TimeProcessor::getWordTime(char *wordTime) {
  if (m_error) {
    return false;
  }
  strcpy(wordTime, m_wordtime);
  return true;
}

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

bool TimeProcessor::setTime0(Timestack *stack) { return true; }

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

  call_times_fcn(getTimeEnumFromSecs(seconds));

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

bool TimeProcessor::update(time_t epochTime) {
  if (m_error) {
    return false;
  }

  m_stack.init();

  if (!getTimeStack(getStack(), epochTime)) {
    return false;
  }

  if (!calcWordTime()) {
    return false;
  }

  return true;
}

bool TimeProcessor::calcWordTime() {
  Timestack *stack;
  TIMESTACK elem;
  char word[12];
  int wordLen{0};
  int cursor{0};

  stack = getStack();

  memset(m_wordtime, 0, sizeof(char) * m_numLetters);

  for (int j = 0; j < stack->getSize(); ++j) {
    if (!stack->get(&elem, j)) {
      return false;
    }
    memset(word, 0, sizeof(word));
    if (!getWord(&elem, word, sizeof(word) / sizeof(char), &wordLen)) {
      return false;
    }
    if (cursor + wordLen < m_numLetters) {
      if (cursor > 0) {
        m_wordtime[cursor++] = ' ';
      }
      memcpy(&m_wordtime[cursor], word, sizeof(char) * wordLen);
      cursor += wordLen;
    } else {
      return false;
    }
  }
  return true;
}
