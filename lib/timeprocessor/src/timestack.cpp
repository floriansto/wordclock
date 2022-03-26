#include <timestack.h>

bool Timestack::push(ClockStr state, bool useDialect) {
  if (m_stacksize >= WORDSTACK_SIZE) {
    return false;
  }
  m_stack[m_stacksize++] = TIMESTACK{useDialect, state};
  return true;
}

bool Timestack::pop(TIMESTACK *elem) {
  if (m_stacksize == 0) {
    elem = nullptr;
    return false;
  }
  *elem = m_stack[--m_stacksize];
  return true;
}

bool Timestack::get(TIMESTACK *elem, int idx) {
  if (idx > m_stacksize - 1) {
    elem = nullptr;
    return false;
  }
  elem = &m_stack[idx];
  return true;
}

int Timestack::getSize() { return m_stacksize; }

void Timestack::init() { m_stacksize = 0; }
