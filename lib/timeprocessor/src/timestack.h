#ifndef __TIMESTACK_H__
#define __TIMESTACK_H__

#define WORDSTACK_SIZE 7

enum class ClockStr;

typedef struct _timestack {
  ClockStr state;
  bool useDialect;
} TIMESTACK;

class Timestack {
public:
  Timestack();
  bool push(TIMESTACK elem);
  bool pop(TIMESTACK *elem);
  void init();
  bool get(TIMESTACK *elem, int idx);
  int getSize();

private:
  int m_stacksize;
  TIMESTACK m_stack[WORDSTACK_SIZE];
};

#endif
