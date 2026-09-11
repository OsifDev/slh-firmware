#pragma once
// HOME SCREEN - app launcher grid with tab bar

struct HomeApp {
    ScreenId screen;
    const char* label;
    uint16_t color;
    int x, y, w, h;
};

static HomeApp HOME_APPS[6] = {
    { SCR_PRICES, "PRICES", 0x07FF,   5,  40, 100, 74 },
    { SCR_SETUP,  "SETUP",  0xFEA0, 110,  40, 100, 74 },
    { SCR_SHOW,   "SHOW",   0xF81F, 215,  40, 100, 74 },
    { SCR_LESSON, "LESSON", 0x07E0,   5, 120, 100, 74 },
    { SCR_MARKET, "MARKET", 0xFD20, 110, 120, 100, 74 },
    { SCR_COUNT,  "WALLET", 0x7BEF, 215, 120, 100, 74 },
};

void drawHomeIcon(int type, int cx, int cy, uint16_t col) {
    switch (type) {
        case 0: {
            tft.drawCircle(cx, cy, 14, col);
            tft.drawCircle(cx, cy, 13, col);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(col, C_PANEL);
            tft.drawString("$", cx, cy - 1, 4);
            tft.setTextDatum(TL_DATUM);
            break;
        }
        case 1: {
            for (int a = 0; a <= 180; a += 15) {
                float rad = (a - 180) * 0.0174533f;
                int x1 = cx + (int)(cos(rad) * 14);
                int y1 = cy + (int)(sin(rad) * 14);
                int x2 = cx + (int)(cos(rad) * 11);
                int y2 = cy + (int)(sin(rad) * 11);
                tft.drawLine(x1, y1, x2, y2, col);
                tft.drawLine(x1, y1, x2, y2, col);
            }
            tft.drawLine(cx, cy, cx + 8, cy - 8, col);
            tft.fillCircle(cx, cy, 3, col);
            break;
        }
        case 2: {
            tft.fillTriangle(cx, cy - 14, cx - 4, cy, cx + 4, cy, col);
            tft.fillTriangle(cx, cy + 14, cx - 4, cy, cx + 4, cy, col);
            tft.fillTriangle(cx - 14, cy, cx, cy - 4, cx, cy + 4, col);
            tft.fillTriangle(cx + 14, cy, cx, cy - 4, cx, cy + 4, col);
            break;
        }
        case 3: {
            tft.drawLine(cx, cy - 12, cx, cy + 12, col);
            tft.drawRect(cx - 14, cy - 10, 13, 22, col);
            tft.drawRect(cx + 1, cy - 10, 13, 22, col);
            tft.drawLine(cx - 11, cy - 5, cx - 4, cy - 5, col);
            tft.drawLine(cx - 11, cy + 1, cx - 4, cy + 1, col);
            tft.drawLine(cx + 4, cy - 5, cx + 11, cy - 5, col);
            tft.drawLine(cx + 4, cy + 1, cx + 11, cy + 1, col);
            break;
        }
        case 4: {
            tft.fillRect(cx - 12, cy + 2, 7, 12, col);
            tft.fillRect(cx - 3, cy - 6, 7, 20, col);
            tft.fillRect(cx + 6, cy - 14, 7, 28, col);
            break;
        }
        case 5: {
            tft.drawRoundRect(cx - 16, cy - 10, 32, 20, 3, col);
            tft.drawRoundRect(cx - 16, cy - 10, 32, 20, 3, col);
            tft.fillRect(cx + 4, cy - 3, 12, 6, col);
            break;
        }
    }
}

void drawHomeScreen() {
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 320, 34, C_PANEL);
    tft.drawFastHLine(0, 34, 320, C_ACCENT);
    drawLogo(20, 17, 13, C_ACCENT);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_ACCENT, C_PANEL);
    tft.drawString("SLH", 42, 17, 4);
    tft.setTextColor(C_DIM, C_PANEL);
    tft.drawString("OS", 92, 17, 2);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_GOLD, C_PANEL);
    tft.drawString("HOME", 312, 17, 2);
    tft.setTextDatum(TL_DATUM);
    for (int i = 0; i < 6; i++) {
        HomeApp& a = HOME_APPS[i];
        tft.fillRoundRect(a.x, a.y, a.w, a.h, 6, C_PANEL);
        tft.drawRoundRect(a.x, a.y, a.w, a.h, 6, a.color);
        int cx = a.x + a.w / 2;
        int cy = a.y + 28;
        drawHomeIcon(i, cx, cy, a.color);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(a.color, C_PANEL);
        tft.drawString(a.label, cx, a.y + a.h - 14, 2);
        tft.setTextDatum(TL_DATUM);
    }
    tft.fillRect(0, 216, 320, 24, C_PANEL);
    tft.drawFastHLine(0, 216, 320, C_ACCENT);
    const char* tabs[4] = { "HOME", "PRICES", "LESSON", "SHOW" };
    uint16_t tabCols[4] = { C_GOLD, C_ACCENT, C_UP, 0xF81F };
    for (int i = 0; i < 4; i++) {
        int tx = i * 80 + 40;
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(tabCols[i], C_PANEL);
        tft.drawString(tabs[i], tx, 228, 2);
        if (i > 0) tft.drawFastVLine(i * 80, 220, 16, C_DIM);
    }
    tft.setTextDatum(TL_DATUM);
}

ScreenId homeTouchHandler(int tx, int ty) {
    if (ty > 216) {
        if (tx < 80) return SCR_HOME;
        if (tx < 160) return SCR_PRICES;
        if (tx < 240) return SCR_LESSON;
        return SCR_SHOW;
    }
    for (int i = 0; i < 6; i++) {
        HomeApp& a = HOME_APPS[i];
        if (tx >= a.x && tx < a.x + a.w && ty >= a.y && ty < a.y + a.h) {
            return a.screen;
        }
    }
    return SCR_COUNT;
}

bool tabBarHomePressed(int tx, int ty) {
    return (ty > 216 && tx < 80);
}