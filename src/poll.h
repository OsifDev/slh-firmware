#pragma once
#include <Arduino.h>

#define SLH_API "https://slh-api-production.up.railway.app"
#define POLL_MS 15000UL

static unsigned long g_lastPoll = 0;
static String g_deviceKey = "";

inline String deviceKey() {
  if (g_deviceKey.length()) return g_deviceKey;
  String m = WiFi.macAddress();
  m.replace(":", "");
  m.toUpperCase();
  g_deviceKey = "ESP_" + m;
  return g_deviceKey;
}

inline void pollServer() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - g_lastPoll < POLL_MS) return;
  g_lastPoll = millis();
  String url = String(SLH_API) + "/api/device/" + deviceKey() + "/state";
  String body = httpGet(url.c_str(), 6000);
  if (body.length() < 5) return;
  DynamicJsonDocument doc(768);
  if (deserializeJson(doc, body)) return;
  const char* scr = doc["screen"] | "";
  if (strlen(scr) > 0) {
    ScreenId t = g_currentScreen;
    if      (!strcmp(scr,"home"))   t = SCR_HOME;
    else if (!strcmp(scr,"prices")) t = SCR_PRICES;
    else if (!strcmp(scr,"market")) t = SCR_MARKET;
    else if (!strcmp(scr,"setup"))  t = SCR_SETUP;
    else if (!strcmp(scr,"show"))   t = SCR_SHOW;
    else if (!strcmp(scr,"lesson")) t = SCR_LESSON;
    if (t != g_currentScreen) { g_currentScreen = t; drawCurrentScreen(); }
  }
}
