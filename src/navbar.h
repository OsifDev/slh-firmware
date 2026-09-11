#pragma once
// Fixed bottom navigation bar - large touch targets

#define NAV_TOP    196
#define NAV_H      44
#define NAV_BTN_W  106

static ScreenId g_prevScreen = SCR_HOME;

void drawNavBar(int pressed = -1) {
    tft.fillRect(0, NAV_TOP, 320, NAV_H, C_PANEL);
    tft.drawFastHLine(0, NAV_TOP, 320, C_ACCENT);
    const char* lbl[3] = { "< BACK", "HOME", "NEXT >" };
    for (int i = 0; i < 3; i++) {
        int x0 = i * NAV_BTN_W;
        int w  = (i == 2) ? (320 - x0) : NAV_BTN_W;
        uint16_t bg = (i == pressed) ? C_ACCENT : C_PANEL;
        uint16_t fg = (i == pressed) ? C_BG : ((i == 1) ? C_GOLD : C_ACCENT);
        tft.fillRect(x0 + 1, NAV_TOP + 2, w - 2, NAV_H - 3, bg);
        if (i > 0) tft.drawFastVLine(x0, NAV_TOP + 6, NAV_H - 12, C_DIM);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(fg, bg);
        tft.drawString(lbl[i], x0 + w / 2, NAV_TOP + NAV_H / 2, 4);
        tft.setTextDatum(TL_DATUM);
    }
}

// returns 0=back 1=home 2=next  -1=not on navbar
int navHit(int tx, int ty) {
    if (ty < NAV_TOP) return -1;
    if (tx < NAV_BTN_W) return 0;
    if (tx < NAV_BTN_W * 2) return 1;
    return 2;
}

void navFeedback(int idx) {
    drawNavBar(idx);
    delay(140);
    drawNavBar(-1);
}
