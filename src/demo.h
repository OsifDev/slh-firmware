#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

#define D_BG     0x0000
#define D_PANEL  0x18E3
#define D_CYAN   0x07FF
#define D_GOLD   0xFEA0
#define D_WHITE  0xFFFF
#define D_DIM    0x7BEF
#define D_GREEN  0x07E0
#define D_RED    0xF9A6

static void dBar(const char* title, int page, int total) {
    tft.fillRect(0, 0, 320, 26, D_PANEL);
    tft.drawFastHLine(0, 26, 320, D_CYAN);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(D_CYAN, D_PANEL);
    tft.drawString(title, 8, 13, 2);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(D_DIM, D_PANEL);
    tft.drawString(String(page) + "/" + String(total), 312, 13, 2);
    tft.setTextDatum(TL_DATUM);
}

static void dNav() {
    tft.fillRect(0, 214, 320, 26, D_PANEL);
    tft.drawFastHLine(0, 214, 320, D_CYAN);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(D_CYAN, D_PANEL);
    tft.drawString("< PREV", 48, 227, 2);
    tft.drawString("NEXT >", 272, 227, 2);
    tft.setTextColor(D_GOLD, D_PANEL);
    tft.drawString("EXIT", 160, 227, 2);
    tft.setTextDatum(TL_DATUM);
}

static void pageTouch() {
    tft.fillRect(0, 27, 320, 187, D_BG);
    dBar("TOUCH TEST - draw with finger", 1, 4);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(D_DIM, D_BG);
    tft.drawString("drag anywhere in this area", 160, 120, 2);
    tft.setTextDatum(TL_DATUM);
    dNav();

    uint16_t x, y;
    unsigned long t0 = millis();
    int hits = 0;
    while (millis() - t0 < 30000) {
        if (tft.getTouch(&x, &y)) {
            if (y > 214) return;
            if (y < 27)  continue;
            hits++;
            uint16_t c = tft.color565(random(60,255), random(60,255), random(60,255));
            tft.fillCircle(x, y, 4, c);
            tft.fillRect(0, 30, 150, 16, D_BG);
            tft.setTextColor(D_GREEN, D_BG);
            tft.drawString("x=" + String(x) + " y=" + String(y) + " n=" + String(hits), 6, 30, 2);
        }
        delay(8);
    }
}

static void pageBench() {
    tft.fillRect(0, 27, 320, 187, D_BG);
    dBar("GRAPHICS BENCHMARK", 2, 4);

    unsigned long t0 = millis();
    int frames = 0;
    while (millis() - t0 < 2500) {
        tft.fillRect(random(0,280), random(35,190), random(10,40), random(10,40),
                     tft.color565(random(255), random(255), random(255)));
        frames++;
    }
    unsigned long el = millis() - t0;
    float rps = frames * 1000.0f / el;

    t0 = millis();
    int lines = 0;
    while (millis() - t0 < 1500) {
        tft.drawLine(random(0,320), random(35,205), random(0,320), random(35,205),
                     tft.color565(random(255), random(255), random(255)));
        lines++;
    }
    float lps = lines * 1000.0f / (millis() - t0);

    tft.fillRect(0, 27, 320, 187, D_BG);
    dBar("GRAPHICS BENCHMARK", 2, 4);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(D_GOLD, D_BG);
    tft.drawString("RESULTS", 160, 50, 4);
    tft.setTextColor(D_WHITE, D_BG);
    tft.drawString(String((int)rps) + " rects/sec", 160, 95, 4);
    tft.drawString(String((int)lps) + " lines/sec", 160, 130, 4);
    tft.setTextColor(D_DIM, D_BG);
    tft.drawString("SPI 40MHz  |  ILI9341  |  320x240", 160, 170, 2);
    tft.setTextDatum(TL_DATUM);
    dNav();

    uint16_t x, y;
    unsigned long w = millis();
    while (millis() - w < 20000) {
        if (tft.getTouch(&x, &y)) return;
        delay(20);
    }
}

static void pageAnim() {
    TFT_eSprite spr = TFT_eSprite(&tft);
    spr.setColorDepth(16);
    if (!spr.createSprite(300, 150)) {
        tft.fillRect(0, 27, 320, 187, D_BG);
        dBar("ANIMATION", 3, 4);
        tft.setTextColor(D_RED, D_BG);
        tft.drawString("not enough RAM for sprite", 20, 110, 2);
        dNav();
        delay(3000);
        return;
    }

    tft.fillRect(0, 27, 320, 187, D_BG);
    dBar("ANIMATION - 60fps sprite", 3, 4);
    dNav();

    const int N = 14;
    float px[N], py[N], vx[N], vy[N];
    uint16_t pc[N];
    for (int i = 0; i < N; i++) {
        px[i] = random(20, 280); py[i] = random(20, 130);
        vx[i] = (random(20, 70) / 20.0f) * (random(2) ? 1 : -1);
        vy[i] = (random(20, 70) / 20.0f) * (random(2) ? 1 : -1);
        pc[i] = tft.color565(random(80,255), random(80,255), random(120,255));
    }

    uint16_t tx, ty;
    unsigned long t0 = millis(), lastF = millis();
    int frames = 0, fps = 0;

    while (millis() - t0 < 30000) {
        if (tft.getTouch(&tx, &ty)) { spr.deleteSprite(); return; }

        spr.fillSprite(D_BG);
        for (int i = 0; i < N; i++) {
            px[i] += vx[i]; py[i] += vy[i];
            if (px[i] < 8 || px[i] > 292) vx[i] = -vx[i];
            if (py[i] < 8 || py[i] > 142) vy[i] = -vy[i];
            for (int j = i + 1; j < N; j++) {
                float dx = px[i]-px[j], dy = py[i]-py[j];
                if (dx*dx + dy*dy < 3600) spr.drawLine(px[i],py[i],px[j],py[j], 0x2124);
            }
        }
        for (int i = 0; i < N; i++) spr.fillCircle(px[i], py[i], 5, pc[i]);

        spr.setTextColor(D_GOLD, D_BG);
        spr.drawString(String(fps) + " FPS", 6, 4, 2);
        spr.pushSprite(10, 32);

        frames++;
        if (millis() - lastF >= 1000) { fps = frames; frames = 0; lastF = millis(); }
    }
    spr.deleteSprite();
}

static void pageInfo() {
    tft.fillRect(0, 27, 320, 187, D_BG);
    dBar("DEVICE INFO", 4, 4);
    dNav();

    int y = 36;
    auto row = [&](const char* k, String v) {
        tft.setTextColor(D_DIM, D_BG);
        tft.drawString(k, 12, y, 2);
        tft.setTextColor(D_WHITE, D_BG);
        tft.drawString(v, 140, y, 2);
        y += 20;
    };

    row("Chip",    String(ESP.getChipModel()) + " r" + String(ESP.getChipRevision()));
    row("CPU",     String(ESP.getCpuFreqMHz()) + " MHz x" + String(ESP.getChipCores()));
    row("Flash",   String(ESP.getFlashChipSize() / 1048576) + " MB");
    row("Free RAM", String(ESP.getFreeHeap() / 1024) + " KB");
    row("MAC",     WiFi.macAddress());
    row("IP",      WiFi.isConnected() ? WiFi.localIP().toString() : String("offline"));
    row("Signal",  WiFi.isConnected() ? String(WiFi.RSSI()) + " dBm" : String("--"));
    row("Uptime",  String(millis() / 1000) + " s");

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(D_GOLD, D_BG);
    tft.drawString("SLH NODE", 160, 200, 2);
    tft.setTextDatum(TL_DATUM);

    uint16_t x, ty;
    unsigned long w = millis();
    while (millis() - w < 30000) {
        if (tft.getTouch(&x, &ty)) return;
        delay(20);
    }
}

inline void runDemoMode() {
    int page = 1;
    while (true) {
        if      (page == 1) pageTouch();
        else if (page == 2) pageBench();
        else if (page == 3) pageAnim();
        else                pageInfo();

        uint16_t x, y;
        unsigned long w = millis();
        bool acted = false;
        while (millis() - w < 15000 && !acted) {
            if (tft.getTouch(&x, &y)) {
                if (y > 214) {
                    if (x < 107)      { page = (page > 1) ? page - 1 : 4; acted = true; }
                    else if (x > 213) { page = (page < 4) ? page + 1 : 1; acted = true; }
                    else              { return; }
                    delay(300);
                } else { acted = true; }
            }
            delay(20);
        }
        if (!acted) page = (page < 4) ? page + 1 : 1;
    }
}
