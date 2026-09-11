#pragma once
#include "lesson_bitmaps.h"

// forward decl - definition is in navbar.h
void drawNavBar(int pressed);

static int           g_lessonPage     = 0;
static unsigned long g_lessonLastFlip = 0;
static const unsigned long LESSON_FLIP_MS = 12000UL;

// Auto-flip between lesson pages. Set to 1 to enable, 0 to freeze.
#define LESSON_AUTOPLAY 0

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
    const uint32_t len = LESSON_SIZES[n];
    static uint16_t rowBuf[320];
    uint32_t si = 0;
    int x = 0, y = 0;
    while (si + 1 < len && y < (int)LESSON_H) {
        uint8_t run = pgm_read_byte(&src[si]);
        uint8_t val = pgm_read_byte(&src[si + 1]);
        si += 2;
        uint16_t c = rgb332_to_565(val);
        while (run--) {
            rowBuf[x++] = c;
            if (x >= (int)LESSON_W) {
                tft.pushImage(0, LESSON_TOP + y, LESSON_W, 1, rowBuf);
                x = 0;
                y++;
                if (y >= (int)LESSON_H) break;
            }
        }
    }
    // Navbar always on top of the lesson image
    drawNavBar(-1);
}

void drawLessonScreen() {
    drawLessonPage(g_lessonPage);
    g_lessonLastFlip = millis();
}

void updateLessonScreen() {
    if (g_currentScreen != SCR_LESSON) return;
#if LESSON_AUTOPLAY
    if (millis() - g_lessonLastFlip >= LESSON_FLIP_MS) {
        g_lessonPage = (g_lessonPage + 1) % (int)LESSON_COUNT;
        drawLessonPage(g_lessonPage);
        g_lessonLastFlip = millis();
    }
#endif
}
