#pragma once
#include "lesson_bitmaps.h"

static int           g_lessonPage     = 0;
static unsigned long g_lessonLastFlip = 0;
static const unsigned long LESSON_FLIP_MS = 12000UL;

static inline uint16_t rgb332_to_565(uint8_t px) {
    uint16_t r = (px >> 5) & 0x07;
    uint16_t g = (px >> 2) & 0x07;
    uint16_t b =  px       & 0x03;
    r = (r << 2) | (r >> 1);
    g = (g << 3) |  g;
    b = (b << 3) | (b << 1) | (b >> 1);
    return (r << 11) | (g << 5) | b;
}

void drawLessonPage(int n) {
    if (n < 0 || n >= (int)LESSON_COUNT) n = 0;
    const uint8_t* src = LESSON_PAGES[n];
    static uint16_t rowBuf[320];
    for (int y = 0; y < (int)LESSON_H; y++) {
        for (int x = 0; x < (int)LESSON_W; x++) {
            rowBuf[x] = rgb332_to_565(pgm_read_byte(&src[y * LESSON_W + x]));
        }
        tft.pushImage(0, LESSON_TOP + y, LESSON_W, 1, rowBuf);
    }
    tft.fillRect(0, 200, 320, 40, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("touch = next screen", 160, 220, 2);
    tft.setTextDatum(TL_DATUM);
}

void drawLessonScreen() {
    drawLessonPage(g_lessonPage);
    g_lessonLastFlip = millis();
}

void updateLessonScreen() {
    if (millis() - g_lessonLastFlip >= LESSON_FLIP_MS) {
        g_lessonPage = (g_lessonPage + 1) % (int)LESSON_COUNT;
        drawLessonPage(g_lessonPage);
        g_lessonLastFlip = millis();
    }
}