#ifndef NONBLOCKINGTIMER_H
#define NONBLOCKINGTIMER_H

#include <Arduino.h>

#define MAX_TIMERS 10

class NonBlockingTimer {
public:
  NonBlockingTimer();

  // Returns timer ID (>= 0), or -1 if no slot available
  int set(void (*callback)(), unsigned long interval);
  int once(void (*callback)(), unsigned long delay);

  // Cancel a timer by its ID
  void cancel(int timerId);

  // Call this in loop()
  void update();

private:
  typedef struct {
    void (*callback)();
    unsigned long interval;
    unsigned long lastRun;
    bool repeating;
    bool active;
  } Timer;

  Timer timers[MAX_TIMERS];
};

#endif