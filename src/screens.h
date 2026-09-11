#pragma once
enum ScreenId { SCR_PRICES = 0, SCR_MARKET, SCR_SETUP, SCR_SHOW, SCR_LESSON, SCR_HOME, SCR_COUNT };
ScreenId g_currentScreen = SCR_HOME;
#define FEE_TAKER_PCT    0.20f
#define FEE_MAKER_PCT    0.15f
#define SPREAD_EST_PCT   0.05f
#define RISK_POSITION    1000.0f
#define STOP_WIDE_PCT    1.0f
#define STOP_TIGHT_PCT   0.5f
#define TARGET_R         2.0f
static inline float coinRangePct(const Coin& c) {
    if (c.low24 <= 0 || c.high24 <= c.low24) return 0;
    return (c.high24 - c.low24) / c.low24 * 100.0f;
}
static inline float coinPosInRange(const Coin& c) {
    if (c.high24 <= c.low24) return 0;
    float p = (c.price - c.low24) / (c.high24 - c.low24) * 100.0f;
    if (p < 0) p = 0; if (p > 100) p = 100;
    return p;
}
static inline uint16_t colorForNetRoom(float net) {
    if (net >= 2.0f) return C_UP;
    if (net >= 1.0f) return C_GOLD;
    return C_DOWN;
}
static inline uint16_t colorForFeesVsMove(float feesPct, float movePct) {
    if (movePct <= 0) return C_DIM;
    float ratio = feesPct / movePct;
    if (ratio >= 0.30f) return C_DOWN;
    if (ratio >= 0.15f) return C_GOLD;
    return C_UP;
}
static void drawScreenBar(ScreenId s, const char* name) {
    tft.fillRect(0, 0, 320, 34, C_PANEL);
    tft.drawFastHLine(0, 34, 320, C_ACCENT);
    drawLogo(20, 17, 13, C_ACCENT);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_ACCENT, C_PANEL);
    tft.drawString("SLH", 42, 17, 4);
    tft.setTextColor(C_DIM, C_PANEL);
    tft.drawString(name, 96, 17, 2);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_GOLD, C_PANEL);
    tft.drawString(String((int)s + 1) + "/" + String((int)SCR_COUNT), 312, 17, 2);
    tft.setTextDatum(TL_DATUM);
}
static void drawTapHint() {
    tft.fillRect(0, 216, 320, 24, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("tap anywhere to change screen", 160, 228, 2);
    tft.setTextDatum(TL_DATUM);
}
void drawMarketScreen() {
    tft.fillScreen(C_BG);
    drawScreenBar(SCR_MARKET, "MARKET");
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("COIN",   12,  44, 2);
    tft.drawString("RANGE",  72,  44, 2);
    tft.drawString("VOL",   156,  44, 2);
    tft.drawString("TRADES",246,  44, 2);
    tft.drawFastHLine(8, 62, 304, C_ACCENT_D);
    int y = 70;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].valid) continue;
        if (coins[i].low24 <= 0 || coins[i].high24 <= coins[i].low24) continue;
        float rng = coinRangePct(coins[i]);
        float pos = coinPosInRange(coins[i]);
        uint16_t col = (rng >= 5.0f) ? C_DOWN : (rng >= 3.0f ? C_GOLD : C_UP);
        tft.setTextColor(C_ACCENT, C_BG);
        tft.drawString(coins[i].symbol, 12, y, 2);
        tft.setTextColor(col, C_BG);
        tft.drawString(String(rng, 2) + "%", 72, y, 2);
        const char* vol = (rng >= 5.0f) ? "HIGH" : (rng >= 3.0f ? "ELEV" : "NORM");
        tft.drawString(vol, 156, y, 2);
        String tr;
        if      (coins[i].trades >= 1000000) tr = String(coins[i].trades / 1000000.0f, 2) + "M";
        else if (coins[i].trades >= 1000)    tr = String(coins[i].trades / 1000.0f, 0)    + "K";
        else                                  tr = String(coins[i].trades);
        tft.drawString(tr, 246, y, 2);
        int bx = 12, by = y + 22, bw = 296, bh = 6;
        tft.drawRect(bx, by, bw, bh, C_DIM);
        int fill = (int)(pos / 100.0f * (bw - 2));
        if (fill > 0) tft.fillRect(bx + 1, by + 1, fill, bh - 2, col);
        int mx = bx + 1 + fill;
        tft.fillTriangle(mx - 3, by - 4, mx + 3, by - 4, mx, by - 1, C_TEXT);
        y += 40;
    }
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("range = (high-low)/low  |  marker = price position", 12, 198, 1);
    drawTapHint();
}
void drawSetupScreen() {
    tft.fillScreen(C_BG);
    drawScreenBar(SCR_SETUP, "SETUP");
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("COIN",    12, 44, 2);
    tft.drawString("RANGE",   68, 44, 2);
    tft.drawString("FEES",   136, 44, 2);
    tft.drawString("NET",    194, 44, 2);
    tft.drawString("VERDICT",252, 44, 2);
    tft.drawFastHLine(8, 62, 304, C_ACCENT_D);
    int y = 68;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].valid) continue;
        if (coins[i].low24 <= 0 || coins[i].high24 <= coins[i].low24) continue;
        float rng  = coinRangePct(coins[i]);
        float fees = FEE_TAKER_PCT + SPREAD_EST_PCT;
        float net  = rng - fees;
        uint16_t netCol = colorForNetRoom(net);
        uint16_t feeCol = colorForFeesVsMove(FEE_TAKER_PCT, rng);
        tft.setTextColor(C_ACCENT, C_BG);
        tft.drawString(coins[i].symbol, 12, y, 2);
        tft.setTextColor(C_TEXT, C_BG);
        tft.drawString(String(rng, 2) + "%", 68, y, 2);
        tft.setTextColor(feeCol, C_BG);
        tft.drawString(String(FEE_TAKER_PCT, 2) + "%", 136, y, 2);
        tft.setTextColor(netCol, C_BG);
        tft.drawString(String(net, 2) + "%", 194, y, 2);
        const char* verdict = (net >= 2.0f) ? "OK" : (net >= 1.0f ? "EDGE" : "NO");
        tft.drawString(verdict, 262, y, 2);
        y += 18;
    }
    tft.drawFastHLine(8, y + 2, 304, C_ACCENT_D);
    y += 10;
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString(String("Fees shown: taker ") + String(FEE_TAKER_PCT, 2) + "% incl " + String(SPREAD_EST_PCT, 2) + "% spread", 12, y, 1);
    y += 10;
    tft.drawString(String("Maker/BNB: ") + String(FEE_MAKER_PCT, 2) + "% - lower with limit orders", 12, y, 1);
    y += 14;
    tft.setTextColor(C_GOLD, C_BG);
    tft.drawString("RISK PER $1,000 POSITION", 12, y, 1);
    y += 12;
    tft.setTextColor(C_TEXT, C_BG);
    tft.drawString("Stop 1.0%  ->  $10",  12, y, 2);
    tft.drawString("Target 2R  ->  $20", 176, y, 2);
    y += 18;
    tft.drawString("Stop 0.5%  ->  $5",   12, y, 2);
    tft.drawString("Target 2R  ->  $10", 176, y, 2);
    y += 22;
    String ctx = "Pos in range:  ";
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].valid) continue;
        ctx += String(coins[i].symbol) + " " + String((int)coinPosInRange(coins[i])) + "%   ";
    }
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString(ctx, 12, y, 1);
    drawTapHint();
}
// =====================================================================
//  SHOW SCREEN — animated showcase of the SLH logo + device vitals
// =====================================================================
static TFT_eSprite* _showSprite     = nullptr;
static float        _showAngle      = 0.0f;
static unsigned long _showLastFrame = 0;
static bool         _showSpriteReady = false;

static void showInit() {
    if (_showSpriteReady) return;
    _showSprite = new TFT_eSprite(&tft);
    if (!_showSprite) return;
    _showSprite->setColorDepth(16);
    if (!_showSprite->createSprite(160, 130)) {
        delete _showSprite;
        _showSprite = nullptr;
        return;
    }
    _showSpriteReady = true;
}

static void showTeardown() {
    if (!_showSpriteReady) return;
    _showSprite->deleteSprite();
    delete _showSprite;
    _showSprite = nullptr;
    _showSpriteReady = false;
}

static void drawShowStatic() {
    tft.fillScreen(C_BG);
    drawScreenBar(SCR_SHOW, "SHOWCASE");

    // ---- bottom: sales pitch ----
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(C_GOLD, C_BG);
    tft.drawString("SLH PRESALE", 160, 176, 2);

    tft.setTextColor(C_TEXT, C_BG);
    tft.drawString("ILS 444", 160, 196, 4);

    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("Smart Layer Hub", 160, 218, 1);

    tft.setTextDatum(TL_DATUM);
}

void drawShowScreen() {
    drawShowStatic();
    showInit();
    _showAngle = 0.0f;
    _showLastFrame = 0;
}

void updateShowScreen() {
    if (!_showSpriteReady) showInit();
    if (!_showSpriteReady) return;

    unsigned long now = millis();
    if (now - _showLastFrame < 33) return;
    _showLastFrame = now;
    _showAngle += 0.07f;
    if (_showAngle > 6.2831853f) _showAngle -= 6.2831853f;

    TFT_eSprite& s = *_showSprite;
    s.fillSprite(C_BG);

    const int cx = 80;
    const int cy = 65;
    const float pulse = (sin(_showAngle * 3.0f) + 1.0f) * 0.5f;

    // --- outer satellite + trail ---
    for (int t = 8; t >= 0; t--) {
        float a = _showAngle * 1.6f - (t * 0.13f);
        int r = 58 + (int)(pulse * 3);
        int dx = cx + (int)(cos(a) * r);
        int dy = cy + (int)(sin(a) * r);
        uint16_t col; int sz;
        if      (t == 0) { col = C_TEXT;     sz = 3; }
        else if (t < 3)  { col = C_ACCENT;   sz = 3; }
        else if (t < 6)  { col = C_ACCENT_D; sz = 2; }
        else             { col = C_PANEL;    sz = 1; }
        s.fillCircle(dx, dy, sz, col);
    }

    // --- outer hexagon (vertices rotate) ---
    const int hexR = 46;
    float hA = _showAngle * 0.4f - 0.5236f;
    int pts[6][2];
    for (int i = 0; i < 6; i++) {
        float a = hA + i * 1.0472f;
        pts[i][0] = cx + (int)(cos(a) * hexR);
        pts[i][1] = cy + (int)(sin(a) * hexR);
    }
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        s.drawLine(pts[i][0], pts[i][1], pts[j][0], pts[j][1], C_ACCENT_D);
        int dotR = 2 + (i % 2);
        s.fillCircle(pts[i][0], pts[i][1], dotR, C_ACCENT);
    }

    // --- inner hexagon, opposite rotation, pulsing ---
    int innerR = 32 + (int)(pulse * 3);
    float iA = -hA;
    int ip[6][2];
    for (int i = 0; i < 6; i++) {
        float a = iA + i * 1.0472f;
        ip[i][0] = cx + (int)(cos(a) * innerR);
        ip[i][1] = cy + (int)(sin(a) * innerR);
    }
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        s.drawLine(ip[i][0], ip[i][1], ip[j][0], ip[j][1], C_GOLD);
    }

    // --- center text ---
    s.setTextDatum(MC_DATUM);
    s.setTextColor(C_TEXT, C_BG);
    s.drawString("SLH", cx, cy - 2, 4);
    s.setTextColor(C_GOLD, C_BG);
    s.drawString("HUB", cx, cy + 22, 1);
    s.setTextDatum(TL_DATUM);

    s.pushSprite(80, 40);
}


#include "lesson.h"
#include "home_screen.h"

void drawCurrentScreen() {
    if (g_currentScreen != SCR_SHOW) showTeardown();
    if (g_currentScreen == SCR_LESSON) g_lessonPage = 0;
    switch (g_currentScreen) {
        case SCR_PRICES: drawAll();          break;
        case SCR_MARKET: drawMarketScreen(); break;
        case SCR_SETUP:  drawSetupScreen();  break;
        case SCR_SHOW:   drawShowScreen();   break;
        case SCR_LESSON: drawLessonScreen(); break;
        case SCR_HOME:   drawHomeScreen();   break;
    }
}
void nextScreen() {
    int _next = ((int)g_currentScreen + 1) % (int)SCR_COUNT; if (_next == SCR_HOME) _next = SCR_PRICES; g_currentScreen = (ScreenId)_next;
    if (g_currentScreen == SCR_PRICES) firstDraw = true;
    drawCurrentScreen();
}





