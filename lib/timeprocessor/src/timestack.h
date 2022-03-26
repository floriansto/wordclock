#ifndef __TIMESTACK_H__
#define __TIMESTACK_H__

#define WORDSTACK_SIZE 7

enum class ClockStr;

typedef struct _timestack {
  bool useDialect;
  ClockStr state;
} TIMESTACK;

class Timestack {
public:
  Timestack(){};
  bool push(ClockStr state, bool useDialect);
  bool pop(TIMESTACK *elem);
  void init();
  bool get(TIMESTACK *elem, int idx);
  int getSize();

private:
  int m_stacksize;
  TIMESTACK m_stack[WORDSTACK_SIZE];
};

#endif
