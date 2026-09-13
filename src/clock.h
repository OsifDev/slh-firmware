#pragma once
#include <time.h>

inline void clockBegin() {
  configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
}

inline String clockTime() {
  struct tm t;
  if (!getLocalTime(&t, 100)) return "--:--";
  char b[8];
  sprintf(b, "%02d:%02d", t.tm_hour, t.tm_min);
  return String(b);
}

inline String clockDate() {
  struct tm t;
  if (!getLocalTime(&t, 100)) return "--/--";
  char b[12];
  sprintf(b, "%02d/%02d", t.tm_mday, t.tm_mon + 1);
  return String(b);
}

inline bool clockReady() {
  struct tm t;
  return getLocalTime(&t, 100);
}
