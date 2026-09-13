#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883

extern WiFiClient   mqNet;
extern PubSubClient mq;
extern String       mqBase;

void mqApply(const String& cmd);

inline String mqDeviceId() {
  String m = WiFi.macAddress();
  m.replace(":", "");
  m.toUpperCase();
  return "ESP_" + m;
}

inline void mqReply(const String& msg) {
  if (!mq.connected()) return;
  mq.publish((mqBase + "/response").c_str(), msg.c_str());
  mq.loop();
  delay(30);
  mq.loop();
  Serial.println("[MQTT] replied: " + msg);
}

String g_mqPending = "";

inline void mqCallback(char* topic, byte* payload, unsigned int len) {
  String cmd = "";
  for (unsigned int i = 0; i < len; i++) cmd += (char)payload[i];
  cmd.trim();
  Serial.println("[MQTT] queued: " + cmd);
  g_mqPending = cmd;
}


inline void mqBegin() {
  mqBase = "slh/esp/" + mqDeviceId();
  mq.setServer(MQTT_HOST, MQTT_PORT);
  mq.setCallback(mqCallback);
  mq.setBufferSize(512);
}

inline void mqLoop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!mq.connected()) {
    static unsigned long lastTry = 0;
    if (millis() - lastTry < 5000) return;
    lastTry = millis();
    String cid = mqDeviceId() + "_" + String(random(10000));
    if (mq.connect(cid.c_str())) {
      mq.subscribe((mqBase + "/command").c_str());
      mq.publish((mqBase + "/response").c_str(), "online");
      Serial.println("[MQTT] connected: " + mqBase);
    }
    return;
  }
  mq.loop();
}
