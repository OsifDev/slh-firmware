#include <WiFiManager.h>
#include "wifi_manager.h"

bool connectWiFi() {
    WiFiManager wm;
    bool res = wm.autoConnect("SLH_Wallet_AP", "12345678");
    Serial.println(res ? "WiFi connected" : "WiFi failed");
    return res;
}
