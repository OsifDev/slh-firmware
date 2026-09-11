#include <TFT_eSPI.h>
#include "tft_display.h"

TFT_eSPI tft = TFT_eSPI();

void initDisplay() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
}

void showReady(const char* ip) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("SLH Kosher Wallet");
    tft.setTextSize(1);
    tft.setCursor(10, 50);
    tft.print("IP: ");
    tft.println(ip);
    tft.setCursor(10, 80);
    tft.println("Status: READY");
}
