#include "nonBlockingTimer.h"

NonBlockingTimer::NonBlockingTimer() {
  for (int i = 0; i < MAX_TIMERS; i++) {
    timers[i].callback = nullptr;
    timers[i].active = false;
  }
}

int NonBlockingTimer::set(void (*callback)(), unsigned long interval) {
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (!timers[i].active) {
      timers[i].callback = callback;
      timers[i].interval = interval;
      timers[i].lastRun = millis();
      timers[i].repeating = true;
      timers[i].active = true;
      return i;
    }
  }
  return -1;  // No available slot
}

int NonBlockingTimer::once(void (*callback)(), unsigned long delay) {
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (!timers[i].active) {
      timers[i].callback = callback;
      timers[i].interval = delay;
      timers[i].lastRun = millis();
      timers[i].repeating = false;
      timers[i].active = true;
      return i;
    }
  }
  return -1;  // No available slot
}

void NonBlockingTimer::cancel(int timerId) {
  if (timerId >= 0 && timerId < MAX_TIMERS) {
    timers[timerId].active = false;
    timers[timerId].callback = nullptr;
  }
}

void NonBlockingTimer::update() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timers[i].active && timers[i].callback) {
      if (now - timers[i].lastRun >= timers[i].interval) {
        timers[i].callback();
        timers[i].lastRun = now;
        if (!timers[i].repeating) {
          timers[i].active = false;
        }
      }
    }
  }
}