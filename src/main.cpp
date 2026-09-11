#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "demo.h"

#define AP_NAME       "SLH-TICKER"
#include "secrets.h"

#define AP_PASSWORD   "slh12345"
#define REFRESH_MS    60000UL
#define SLH_ENABLED   1
#define SLH_FIXED_ILS 444.0f

uint16_t touchCal[5] = { 1169, 2749, 837, 2958, 7 };

#define C_BG        0x0000
#define C_PANEL     0x18E3
#define C_ACCENT    0x07FF
#define C_ACCENT_D  0x0410
#define C_TEXT      0xFFFF
#define C_DIM       0x7BEF
#define C_UP        0x07E0
#define C_DOWN      0xF9A6
#define C_GOLD      0xFEA0

TFT_eSPI tft = TFT_eSPI();

struct Coin {
    const char* symbol;
    const char* geckoId;
    const char* binanceSym;
    float       price;
    float       change24h;
    bool        valid;
};

Coin coins[] = {
    { "BTC", "bitcoin",     "BTCUSDT", 0, 0, false },
    { "ETH", "ethereum",    "ETHUSDT", 0, 0, false },
    { "BNB", "binancecoin", "BNBUSDT", 0, 0, false },
#if SLH_ENABLED
    { "SLH", nullptr,       nullptr,   0, 0, false },
#endif
};
const int COIN_COUNT = sizeof(coins) / sizeof(coins[0]);

unsigned long lastRefresh   = 0;
unsigned long lastSuccessMs = 0;
String        sourceLabel   = "--";
bool          firstDraw     = true;

String fmtPrice(float p) {
    if (p <= 0)      return "--";
    if (p >= 1000)   return String(p, 0);
    if (p >= 10)     return String(p, 2);
    if (p >= 0.01)   return String(p, 4);
    return String(p, 6);
}

String fmtAge(unsigned long ms) {
    if (lastSuccessMs == 0) return "never";
    unsigned long s = (millis() - ms) / 1000UL;
    if (s < 60)   return String(s) + "s ago";
    if (s < 3600) return String(s / 60) + "m ago";
    return String(s / 3600) + "h ago";
}

String httpGet(const char* url, uint16_t timeoutMs = 8000) {
    if (WiFi.status() != WL_CONNECTED) return "";
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setTimeout(timeoutMs);
    http.setConnectTimeout(timeoutMs);
    if (!http.begin(client, url)) return "";
    int code = http.GET();
    String body = "";
    if (code == HTTP_CODE_OK) body = http.getString();
    else Serial.printf("[HTTP] %d\n", code);
    http.end();
    return body;
}

void drawLogo(int cx, int cy, int r, uint16_t color) {
    float pts[6][2];
    for (int i = 0; i < 6; i++) {
        float a = (60.0f * i - 30.0f) * DEG_TO_RAD;
        pts[i][0] = cx + r * cos(a);
        pts[i][1] = cy + r * sin(a);
    }
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        tft.drawLine(pts[i][0], pts[i][1], pts[j][0], pts[j][1], color);
    }
    for (int i = 0; i < 6; i++) {
        int j = (i + 1) % 6;
        int x1 = cx + (pts[i][0] - cx) * 0.72f;
        int y1 = cy + (pts[i][1] - cy) * 0.72f;
        int x2 = cx + (pts[j][0] - cx) * 0.72f;
        int y2 = cy + (pts[j][1] - cy) * 0.72f;
        tft.drawLine(x1, y1, x2, y2, C_ACCENT_D);
    }
    for (int i = 0; i < 6; i++) tft.fillCircle(pts[i][0], pts[i][1], 2, color);
}

void drawSplash() {
    tft.fillScreen(C_BG);
    drawLogo(160, 88, 34, C_ACCENT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_ACCENT, C_BG);
    tft.drawString("SLH", 160, 88, 4);
    tft.setTextColor(C_TEXT, C_BG);
    tft.drawString("CRYPTO TICKER", 160, 148, 4);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("Smart Layer Hub", 160, 174, 2);
    tft.drawFastHLine(80, 192, 160, C_ACCENT_D);
    tft.fillCircle(80,  192, 2, C_ACCENT);
    tft.fillCircle(240, 192, 2, C_ACCENT);
    tft.setTextDatum(TL_DATUM);
}

void drawStatus(const String& msg, uint16_t color = C_DIM) {
    tft.fillRect(0, 216, 320, 24, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(color, C_BG);
    tft.drawString(msg, 160, 228, 2);
    tft.setTextDatum(TL_DATUM);
}

void drawFrame() {
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 320, 34, C_PANEL);
    tft.drawFastHLine(0, 34, 320, C_ACCENT);
    drawLogo(20, 17, 13, C_ACCENT);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_ACCENT, C_PANEL);
    tft.drawString("SLH", 42, 17, 4);
    tft.setTextColor(C_DIM, C_PANEL);
    tft.drawString("CRYPTO", 92, 17, 2);
    tft.setTextDatum(TL_DATUM);
}

void drawRow(int index, const Coin& c) {
    const int top = 42 + index * 43;
    const int h   = 39;
    tft.fillRoundRect(6, top, 308, h, 4, C_PANEL);
    uint16_t stripe = (strcmp(c.symbol, "SLH") == 0) ? C_GOLD : C_ACCENT;
    tft.fillRect(6, top, 3, h, stripe);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(stripe, C_PANEL);
    tft.drawString(c.symbol, 18, top + h / 2, 4);
    tft.setTextDatum(MR_DATUM);
    if (c.valid) {
        tft.setTextColor(C_TEXT, C_PANEL);
        tft.drawString(((strcmp(c.symbol,"SLH")==0) ? String("ILS ") : String("$")) + fmtPrice(c.price), 240, top + h / 2, 4);
    } else {
        tft.setTextColor(C_DIM, C_PANEL);
        tft.drawString("--", 240, top + h / 2, 4);
    }
    if (strcmp(c.symbol, "SLH") == 0) {
        tft.drawRoundRect(246, top + 10, 64, 19, 4, C_GOLD);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(C_GOLD, C_PANEL);
        tft.drawString("PRESALE", 278, top + 19, 2);
        tft.setTextDatum(TL_DATUM);
    } else if (c.valid && c.change24h != 0) {
        uint16_t col = (c.change24h >= 0) ? C_UP : C_DOWN;
        String s = (c.change24h >= 0 ? "+" : "") + String(c.change24h, 2) + "%";
        tft.setTextColor(col, C_PANEL);
        tft.drawString(s, 306, top + h / 2, 2);
        int ty = top + h / 2;
        if (c.change24h >= 0) tft.fillTriangle(250, ty + 4, 256, ty - 4, 262, ty + 4, col);
        else                  tft.fillTriangle(250, ty - 4, 256, ty + 4, 262, ty - 4, col);
    }
    tft.setTextDatum(TL_DATUM);
}

void drawAll() {
    if (firstDraw) { drawFrame(); firstDraw = false; }
    for (int i = 0; i < COIN_COUNT; i++) drawRow(i, coins[i]);
    String line = sourceLabel + "  |  " + fmtAge(lastSuccessMs);
    drawStatus(line, (WiFi.status() == WL_CONNECTED) ? C_DIM : C_DOWN);
}

bool fetchCoinGecko() {
    String url = "https://api.coingecko.com/api/v3/simple/price?ids=";
    bool first = true;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].geckoId) continue;
        if (!first) url += ",";
        url += coins[i].geckoId;
        first = false;
    }
    url += "&vs_currencies=usd&include_24hr_change=true";
    String body = httpGet(url.c_str());
    if (body.length() == 0) return false;
    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, body)) return false;
    int got = 0;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].geckoId) continue;
        JsonObject o = doc[coins[i].geckoId];
        if (o.isNull()) continue;
        float p = o["usd"] | 0.0f;
        if (p <= 0) continue;
        coins[i].price     = p;
        coins[i].change24h = o["usd_24h_change"] | 0.0f;
        coins[i].valid     = true;
        got++;
    }
    return got > 0;
}

bool fetchBinance() {
    int got = 0;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].binanceSym) continue;
        String url = "https://api.binance.com/api/v3/ticker/24hr?symbol=";
        url += coins[i].binanceSym;
        String body = httpGet(url.c_str());
        if (body.length() == 0) continue;
        DynamicJsonDocument doc(3072);
        if (deserializeJson(doc, body)) continue;
        float p = doc["lastPrice"] | 0.0f;
        if (p <= 0) continue;
        coins[i].price     = p;
        coins[i].change24h = doc["priceChangePercent"] | 0.0f;
        coins[i].valid     = true;
        got++;
        delay(120);
    }
    return got > 0;
}

#if SLH_ENABLED
bool fetchSLH() {
    for (int i = 0; i < COIN_COUNT; i++) {
        if (strcmp(coins[i].symbol, "SLH") != 0) continue;
        coins[i].price     = SLH_FIXED_ILS;
        coins[i].change24h = 0;
        coins[i].valid     = true;
        return true;
    }
    return false;
}

#endif

void refreshPrices() {
    drawStatus("updating...", C_ACCENT);
    bool ok = false;
    if (fetchCoinGecko())    { sourceLabel = "CoinGecko"; ok = true; }
    else if (fetchBinance()) { sourceLabel = "Binance";   ok = true; }
    else                     { sourceLabel = "offline"; }
#if SLH_ENABLED
    if (fetchSLH() && ok) sourceLabel += " + SLH";
#endif
    if (ok) lastSuccessMs = millis();
    lastRefresh = millis();
    drawAll();
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[SLH] Crypto Ticker booting");
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    tft.setTouch(touchCal);
    drawSplash();
    delay(1200);
    drawStatus("connecting to WiFi...", C_ACCENT);
    WiFi.persistent(false);
    WiFi.disconnect(true, true);
    delay(300);
    WiFi.mode(WIFI_STA);
    Serial.println("[SLH] scanning networks...");
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        Serial.printf("  [%s]  rssi=%d  enc=%d\n",
            WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i));
    }
    Serial.printf("[SLH] %d networks found\n", n);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[SLH] WiFi FAILED - check SSID/password in main.cpp");
        drawStatus("WiFi FAILED", C_DOWN);
        delay(4000);
        ESP.restart();
    }
    Serial.print("[SLH] IP: ");
    Serial.println(WiFi.localIP());
    drawStatus(WiFi.localIP().toString(), C_UP);
    delay(900);
    refreshPrices();
}

void loop() {
    uint16_t tx, ty;
    if (tft.getTouch(&tx, &ty)) {
        if (ty < 34) {
            runDemoMode();
            firstDraw = true;
            drawAll();
        } else {
            refreshPrices();
        }
        delay(400);
        return;
    }
    if (millis() - lastRefresh >= REFRESH_MS) {
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.reconnect();
            delay(3000);
        }
        refreshPrices();
    }
    static unsigned long lastTick = 0;
    if (millis() - lastTick > 5000) {
        lastTick = millis();
        String line = sourceLabel + "  |  " + fmtAge(lastSuccessMs);
        drawStatus(line, WiFi.status() == WL_CONNECTED ? C_DIM : C_DOWN);
    }
    delay(50);
}
