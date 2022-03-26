#include <timestack.h>

bool Timestack::push(TIMESTACK elem) {
  if (m_stacksize >= WORDSTACK_SIZE) {
    return false;
  }
  m_stack[m_stacksize++] = elem;
  return true;
}

Timestack::Timestack() { m_stacksize = 0; }

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
  *elem = m_stack[idx];
  return true;
}

int Timestack::getSize() { return m_stacksize; }

void Timestack::init() { m_stacksize = 0; }
