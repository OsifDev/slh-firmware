#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "rawtouch.h"
SPIClass rtSPI(VSPI);
int rtCal[4] = {3204, 740, 3095, 1355};
#include "demo.h"
#include <WebServer.h>
#include <Update.h>
WebServer server(80);

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

// --- screensaver state ---
static unsigned long g_lastActivity  = 0;
static bool          g_saverOn       = true;
static bool          g_saverActive   = false;
static unsigned long g_saverLastFlip = 0;
static const unsigned long SAVER_IDLE_MS  = 90000UL;
static const unsigned long SAVER_CYCLE_MS = 20000UL;

static bool          g_touchWasDown  = false;
static unsigned long g_lastTouchMs   = 0;
static const unsigned long TOUCH_COOLDOWN_MS = 350UL;

TFT_eSPI tft = TFT_eSPI();

struct Coin {
    const char* symbol;
    const char* geckoId;
    const char* binanceSym;
    float       price;
    float       change24h;
    float       high24;
    float       low24;
    float       volume;
    int         trades;
    bool        valid;
};

Coin coins[] = {
    { "BTC", "bitcoin",     "BTCUSDT", 0, 0, 0, 0, 0, 0, false },
    { "ETH", "ethereum",    "ETHUSDT", 0, 0, 0, 0, 0, 0, false },
    { "BNB", "binancecoin", "BNBUSDT", 0, 0, 0, 0, 0, 0, false },
#if SLH_ENABLED
    { "SLH", nullptr,       nullptr,   0, 0, 0, 0, 0, 0, false },
#endif
};
const int COIN_COUNT = sizeof(coins) / sizeof(coins[0]);

#define SPARK_N 24
float sparkData[4][SPARK_N];
bool  sparkReady[4] = {false, false, false, false};
void drawSparkline(int i, int x, int y, int w, int h);

unsigned long lastRefresh   = 0;
unsigned long lastSuccessMs = 0;
String        sourceLabel   = "--";
bool          firstDraw     = true;

// 0=prices 1=market 2=setup
int  screenMode = 0;
int  setupCoin  = 0;
const char* SCREEN_NAMES[3] = {"prices", "market", "setup"};
void drawMarket();
void drawSetup();

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
    tft.drawString("PRICES", 96, 17, 2);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_GOLD, C_PANEL);
    tft.drawString("1/3", 312, 17, 2);
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
    drawSparkline(index, 62, top + 8, 72, 23);

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
    if (screenMode == 1) { drawMarket(); return; }
    if (screenMode == 2) { drawSetup();  return; }
    if (firstDraw) { drawFrame(); firstDraw = false; }
    for (int i = 0; i < COIN_COUNT; i++) drawRow(i, coins[i]);
    String line = sourceLabel + "  |  " + fmtAge(lastSuccessMs);
    drawStatus(line, (WiFi.status() == WL_CONNECTED) ? C_DIM : C_DOWN);
}

#include "screens.h"

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

void fetchSparklines() {
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].binanceSym) continue;
        String url = "https://api.binance.com/api/v3/klines?symbol=";
        url += coins[i].binanceSym;
        url += "&interval=1h&limit=";
        url += String(SPARK_N);
        String body = httpGet(url.c_str(), 9000);
        if (body.length() < 20) continue;

        // klines come back as arrays; index 4 of each is the close price
        int idx = 0, pos = 0;
        while (idx < SPARK_N) {
            int open = body.indexOf('[', pos);
            if (open < 0) break;
            int close = body.indexOf(']', open);
            if (close < 0) break;
            String row = body.substring(open + 1, close);
            int field = 0, from = 0;
            String val = "";
            for (int k = 0; k <= row.length(); k++) {
                if (k == row.length() || row[k] == ',') {
                    if (field == 4) { val = row.substring(from, k); break; }
                    field++; from = k + 1;
                }
            }
            val.replace("\"", "");
            float p = val.toFloat();
            if (p > 0) sparkData[i][idx++] = p;
            pos = close + 1;
        }
        if (idx == SPARK_N) sparkReady[i] = true;
        delay(150);
    }
}

void drawSparkline(int i, int x, int y, int w, int h) {
    if (!sparkReady[i]) return;
    float lo = sparkData[i][0], hi = sparkData[i][0];
    for (int k = 1; k < SPARK_N; k++) {
        if (sparkData[i][k] < lo) lo = sparkData[i][k];
        if (sparkData[i][k] > hi) hi = sparkData[i][k];
    }
    float range = hi - lo;
    if (range <= 0) range = 1;

    bool up = sparkData[i][SPARK_N-1] >= sparkData[i][0];
    uint16_t col = up ? C_UP : C_DOWN;

    int prevX = x, prevY = y + h - (int)((sparkData[i][0] - lo) / range * h);
    for (int k = 1; k < SPARK_N; k++) {
        int cx = x + (k * w) / (SPARK_N - 1);
        int cy = y + h - (int)((sparkData[i][k] - lo) / range * h);
        tft.drawLine(prevX, prevY, cx, cy, col);
        prevX = cx; prevY = cy;
    }
    tft.fillCircle(prevX, prevY, 2, col);
}

void calibrateTouch() {
    uint16_t cal[5];
    tft.fillScreen(C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_ACCENT, C_BG);
    tft.drawString("TOUCH CALIBRATION", 160, 90, 4);
    tft.setTextColor(C_TEXT, C_BG);
    tft.drawString("tap each corner arrow", 160, 130, 2);
    tft.setTextDatum(TL_DATUM);
    delay(2000);

    tft.calibrateTouch(cal, C_ACCENT, C_BG, 15);

    Serial.println("\n=== COPY THIS LINE INTO main.cpp ===");
    Serial.print("uint16_t touchCal[5] = { ");
    for (int i = 0; i < 5; i++) {
        Serial.print(cal[i]);
        if (i < 4) Serial.print(", ");
    }
    Serial.println(" };");
    Serial.println("====================================\n");

    tft.setTouch(cal);
    tft.fillScreen(C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_UP, C_BG);
    tft.drawString("CALIBRATED", 160, 100, 4);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("values printed to serial", 160, 140, 2);
    tft.setTextDatum(TL_DATUM);
    delay(3000);
}

String buildStateJson() {
    String j = "{";
    j += "\"device\":{";
    j += "\"mac\":\"" + WiFi.macAddress() + "\",";
    j += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    j += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    j += "\"uptime_s\":" + String(millis()/1000) + ",";
    j += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    j += "\"chip\":\"" + String(ESP.getChipModel()) + "\"";
    j += "},";
    j += "\"screen\":\"" + String(SCREEN_NAMES[screenMode]) + "\",";
    j += "\"source\":\"" + sourceLabel + "\",";
    j += "\"age_s\":" + String(lastSuccessMs ? (millis()-lastSuccessMs)/1000 : 0) + ",";
    j += "\"touch\":{";
    { uint16_t tx, ty; bool hit = tft.getTouch(&tx, &ty);
      j += "\"pressed\":"; j += (hit ? "true" : "false");
      j += ",\"x\":" + String(hit ? tx : 0) + ",\"y\":" + String(hit ? ty : 0);
      j += ",\"cal\":[" + String(rtCal[0]) + "," + String(rtCal[1]) + "," + String(rtCal[2]) + "," + String(rtCal[3]) + "]"; }
    j += "},";
    j += "\"rows\":[";
    for (int i = 0; i < COIN_COUNT; i++) {
        if (i) j += ",";
        j += "{\"symbol\":\"" + String(coins[i].symbol) + "\"";
        j += ",\"price\":" + String(coins[i].price, 6);
        j += ",\"change_24h\":" + String(coins[i].change24h, 3);
        j += ",\"valid\":"; j += (coins[i].valid ? "true" : "false");
        j += ",\"high24\":" + String(coins[i].high24, 2);
        j += ",\"low24\":" + String(coins[i].low24, 2);
        j += ",\"volume\":" + String(coins[i].volume, 1);
        j += ",\"trades\":" + String(coins[i].trades);
        if (coins[i].high24 > coins[i].low24) {
            float rng = coins[i].high24 - coins[i].low24;
            j += ",\"range_pct\":" + String(rng / coins[i].low24 * 100.0f, 2);
            j += ",\"pos_in_range\":" + String((coins[i].price - coins[i].low24) / rng * 100.0f, 1);
        } else { j += ",\"range_pct\":0,\"pos_in_range\":0"; }

        j += ",\"spark\":";
        if (sparkReady[i]) {
            j += "[";
            for (int k = 0; k < SPARK_N; k++) { j += String(sparkData[i][k], 2); if (k < SPARK_N-1) j += ","; }
            j += "]";
        } else j += "null";
        j += "}";
    }
    j += "]}";
    return j;
}

void setupServer() {
    server.on("/", [](){ server.send(200, "application/json", buildStateJson()); });
    server.on("/state", [](){ server.send(200, "application/json", buildStateJson()); });
    server.on("/refresh", [](){ server.send(200, "application/json", "{\"ok\":true}"); lastRefresh = 0; });
    server.on("/screen", [](){
        String to = server.arg("to");
        auto nameOf = [](ScreenId s) -> String {
            switch (s) {
                case SCR_PRICES: return "prices";
                case SCR_MARKET: return "market";
                case SCR_SETUP:  return "setup";
                case SCR_SHOW:   return "show";
                case SCR_HOME:   return "home";
                case SCR_LESSON: return "lesson";
                default:         return "unknown";
            }
        };
        if (to.length() == 0) {
            String r = "{\"current\":\"" + nameOf(g_currentScreen)
                     + "\",\"all\":[\"prices\",\"market\",\"setup\",\"show\",\"lesson\"]}";
            server.send(200, "application/json", r);
            return;
        }
        ScreenId target = g_currentScreen;
        if      (to == "prices") target = SCR_PRICES;
        else if (to == "market") target = SCR_MARKET;
        else if (to == "setup")  target = SCR_SETUP;
        else if (to == "show")   target = SCR_SHOW;
        else if (to == "home")   target = SCR_HOME;
        else if (to == "lesson") { target = SCR_LESSON; g_lessonPage = 0; }
        else if (to == "next")   target = (ScreenId)(((int)g_currentScreen + 1) % (int)SCR_COUNT);
        else { server.send(400, "application/json", "{\"error\":\"unknown\"}"); return; }
        g_currentScreen = target;
        if (target == SCR_PRICES) firstDraw = true;
        drawCurrentScreen();
        String r = "{\"ok\":true,\"screen\":\"" + nameOf(target) + "\"}";
        server.send(200, "application/json", r);
    });
    server.on("/lesson", [](){
        String p = server.arg("page");
        if (p.length() == 0) {
            server.send(200, "application/json",
                "{\"page\":" + String(g_lessonPage) + ",\"count\":" + String(LESSON_COUNT) + "}");
            return;
        }
        int n = p.toInt();
        if (n < 0 || n >= LESSON_COUNT) {
            server.send(400, "application/json", "{\"error\":\"range\"}");
            return;
        }
        g_lessonPage = n;
        g_currentScreen = SCR_LESSON;
        drawCurrentScreen();
        server.send(200, "application/json",
            "{\"ok\":true,\"page\":" + String(n) + "}");
    });    server.on("/backlight", [](){
        String p = server.arg("pct");
        static uint8_t blPct = 100;
        if (p.length() == 0) {
            server.send(200, "application/json", "{\"pct\":" + String(blPct) + "}");
            return;
        }
        int v = p.toInt();
        if (v < 5) v = 5;
        if (v > 100) v = 100;
        blPct = (uint8_t)v;
        ledcWrite(0, (v * 255) / 100);
        server.send(200, "application/json", "{\"ok\":true,\"pct\":" + String(v) + "}");
    });    server.on("/touch", [](){
        uint16_t rx, ry, rz, sx, sy;
        bool valid = rtRaw(&rx, &ry, &rz);
        bool mapped = rtTouch(&sx, &sy);
        String r = "{\"pressed\":"; r += (valid ? "true" : "false");
        r += ",\"raw\":{\"x\":" + String(rx) + ",\"y\":" + String(ry) + ",\"z\":" + String(rz) + "}";
        r += ",\"screen\":{\"x\":" + String(mapped?sx:0) + ",\"y\":" + String(mapped?sy:0) + "}";
        r += ",\"cal\":[" + String(rtCal[0]) + "," + String(rtCal[1]) + "," + String(rtCal[2]) + "," + String(rtCal[3]) + "]}";
        server.send(200, "application/json", r);
    });
    server.on("/rawdump", [](){
        String r = "{\"samples\":[";
        rtSPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        for (int n = 0; n < 10; n++) {
            uint16_t z = rtRead(0xB1);
            rtRead(0x91);
            uint16_t x = rtRead(0x91);
            rtRead(0xD1);
            uint16_t y = rtRead(0xD1);
            if (n) r += ",";
            r += "{\"x\":" + String(x) + ",\"y\":" + String(y) + ",\"z\":" + String(z) + "}";
            delayMicroseconds(300);
        }
        rtSPI.endTransaction();
        r += "]}";
        server.send(200, "application/json", r);
    });
    server.on("/screen", [](){
        if (server.hasArg("n")) { screenMode = server.arg("n").toInt() % 3; firstDraw = true; drawAll(); }
        server.send(200, "application/json", "{\"screen\":\"" + String(SCREEN_NAMES[screenMode]) + "\"}");
    });
    server.on("/setcal", [](){
        if (server.hasArg("x0")) rtCal[0] = server.arg("x0").toInt();
        if (server.hasArg("x1")) rtCal[1] = server.arg("x1").toInt();
        if (server.hasArg("y0")) rtCal[2] = server.arg("y0").toInt();
        if (server.hasArg("y1")) rtCal[3] = server.arg("y1").toInt();
        String r = "{\"cal\":[" + String(rtCal[0]) + "," + String(rtCal[1]) + "," + String(rtCal[2]) + "," + String(rtCal[3]) + "]}";
        server.send(200, "application/json", r);
    });
    server.on("/ota", HTTP_GET, [](){
        server.send(200, "text/html",
          "<html><body style='font-family:sans-serif;background:#111;color:#eee;padding:30px'>"
          "<h2>SLH Firmware Update</h2>"
          "<form method='POST' action='/update' enctype='multipart/form-data'>"
          "<input type='file' name='f'><br><br>"
          "<input type='submit' value='Upload firmware.bin'></form></body></html>");
    });
    server.on("/update", HTTP_POST, [](){
        bool ok = !Update.hasError();
        server.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
        delay(600);
        if (ok) ESP.restart();
    }, [](){
        HTTPUpload& up = server.upload();
        if (up.status == UPLOAD_FILE_START) {
            Serial.printf("[OTA] start: %s\n", up.filename.c_str());
            tft.fillScreen(C_BG);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(C_ACCENT, C_BG);
            tft.drawString("FIRMWARE UPDATE", 160, 100, 4);
            tft.setTextDatum(TL_DATUM);
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
        } else if (up.status == UPLOAD_FILE_WRITE) {
            Update.write(up.buf, up.currentSize);
            static int last = -1;
            int pct = (Update.progress() * 100) / 1100000;
            if (pct != last && pct <= 100) {
                last = pct;
                tft.fillRect(40, 140, 240, 18, C_PANEL);
                tft.fillRect(40, 140, (240 * pct) / 100, 18, C_ACCENT);
            }
        } else if (up.status == UPLOAD_FILE_END) {
            if (Update.end(true)) Serial.printf("[OTA] done: %u bytes\n", up.totalSize);
            else Update.printError(Serial);
        }
    });
    server.begin();
    Serial.println("[SLH] HTTP API on port 80");
}

void fetchStats() {
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].binanceSym) continue;
        String url = "https://api.binance.com/api/v3/ticker/24hr?symbol=";
        url += coins[i].binanceSym;
        String body = httpGet(url.c_str(), 8000);
        if (body.length() < 20) continue;
        DynamicJsonDocument doc(3072);
        if (deserializeJson(doc, body)) continue;
        coins[i].high24  = String((const char*)(doc["highPrice"] | "0")).toFloat();
        coins[i].low24   = String((const char*)(doc["lowPrice"]  | "0")).toFloat();
        coins[i].volume  = String((const char*)(doc["volume"]    | "0")).toFloat();
        coins[i].trades  = doc["count"]      | 0;
        delay(120);
    }
}

void drawMarket() {
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 320, 28, C_PANEL);
    tft.drawFastHLine(0, 28, 320, C_ACCENT);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_ACCENT, C_PANEL);
    tft.drawString("MARKET", 10, 14, 4);
    tft.setTextColor(C_DIM, C_PANEL);
    tft.drawString("24h context", 110, 14, 2);
    tft.setTextDatum(TL_DATUM);

    int y = 36;
    for (int i = 0; i < COIN_COUNT; i++) {
        if (!coins[i].binanceSym || coins[i].high24 <= coins[i].low24) continue;
        float rng = coins[i].high24 - coins[i].low24;
        float pct = rng / coins[i].low24 * 100.0f;
        int pos = (int)((coins[i].price - coins[i].low24) / rng * 100.0f);
        if (pos < 0) pos = 0; if (pos > 100) pos = 100;

        tft.setTextColor(C_ACCENT, C_BG);
        tft.drawString(coins[i].symbol, 10, y, 4);

        uint16_t vc = (pct > 4) ? C_GOLD : ((pct > 2) ? C_TEXT : C_DIM);
        tft.setTextDatum(MR_DATUM);
        tft.setTextColor(vc, C_BG);
        tft.drawString(String(pct, 2) + "% day", 310, y + 10, 2);
        tft.setTextDatum(TL_DATUM);

        int by = y + 26;
        tft.drawRect(10, by, 300, 12, C_PANEL);
        tft.fillRect(11, by + 1, 298, 10, C_PANEL);
        int mx = 11 + (298 * pos) / 100;
        uint16_t pc = (pos > 70) ? C_UP : ((pos < 30) ? C_DOWN : C_ACCENT);
        tft.fillRect(mx - 2, by, 4, 12, pc);

        tft.setTextColor(C_DIM, C_BG);
        tft.drawString(String(pos) + "% of range", 10, by + 15, 2);
        tft.setTextDatum(MR_DATUM);
        tft.drawString("L " + String(coins[i].low24, 0) + "  H " + String(coins[i].high24, 0), 310, by + 20, 2);
        tft.setTextDatum(TL_DATUM);

        y += 66;
    }
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("tap to change screen", 160, 232, 2);
    tft.setTextDatum(TL_DATUM);
}

void drawSetup() {
    Coin& c = coins[setupCoin];
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 320, 28, C_PANEL);
    tft.drawFastHLine(0, 28, 320, C_ACCENT);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_ACCENT, C_PANEL);
    tft.drawString("SETUP", 10, 14, 4);
    tft.setTextColor(C_GOLD, C_PANEL);
    tft.drawString(c.symbol, 90, 14, 4);
    tft.setTextColor(C_DIM, C_PANEL);
    tft.drawString("risk per $1000", 150, 14, 2);
    tft.setTextDatum(TL_DATUM);

    if (c.high24 <= c.low24) {
        tft.setTextColor(C_DIM, C_BG);
        tft.drawString("no data yet", 20, 100, 4);
        return;
    }

    float rng  = c.high24 - c.low24;
    float move = rng / c.low24 * 100.0f;
    int   pos  = (int)((c.price - c.low24) / rng * 100.0f);
    if (pos < 0) pos = 0; if (pos > 100) pos = 100;

    float spotFee = 0.20f;
    float futFee  = 0.10f;
    int spotShare = (int)(spotFee / move * 100.0f);
    int futShare  = (int)(futFee  / move * 100.0f);

    int y = 36;
    tft.setTextColor(C_DIM, C_BG);  tft.drawString("day move", 12, y, 2);
    tft.setTextColor(C_TEXT, C_BG); tft.drawString(String(move, 2) + "%", 120, y, 2);
    y += 20;

    tft.setTextColor(C_DIM, C_BG);  tft.drawString("spot fees", 12, y, 2);
    tft.setTextColor(spotShare > 25 ? C_DOWN : C_TEXT, C_BG);
    tft.drawString("0.20%  = " + String(spotShare) + "% of move", 120, y, 2);
    y += 20;

    tft.setTextColor(C_DIM, C_BG);  tft.drawString("futures", 12, y, 2);
    tft.setTextColor(futShare > 25 ? C_DOWN : C_TEXT, C_BG);
    tft.drawString("0.10%  = " + String(futShare) + "% of move", 120, y, 2);
    y += 26;

    tft.drawFastHLine(12, y, 296, C_PANEL);
    y += 8;

    tft.setTextColor(C_DIM, C_BG);  tft.drawString("stop 1%", 12, y, 2);
    tft.setTextColor(C_TEXT, C_BG); tft.drawString("risk $10", 120, y, 2);
    tft.setTextColor(C_DIM, C_BG);  tft.drawString("target 2:1 = $20", 210, y, 2);
    y += 20;
    tft.setTextColor(C_DIM, C_BG);  tft.drawString("stop 2%", 12, y, 2);
    tft.setTextColor(C_TEXT, C_BG); tft.drawString("risk $20", 120, y, 2);
    tft.setTextColor(C_DIM, C_BG);  tft.drawString("target 2:1 = $40", 210, y, 2);
    y += 26;

    tft.drawFastHLine(12, y, 296, C_PANEL);
    y += 8;

    if (pos > 70) {
        tft.setTextColor(C_GOLD, C_BG);
        tft.drawString("price near DAY HIGH (" + String(pos) + "%)", 12, y, 2);
        y += 18;
        tft.setTextColor(C_DIM, C_BG);
        tft.drawString("long needs the high to break", 12, y, 2);
        y += 16;
        tft.drawString("short needs it to hold", 12, y, 2);
    } else if (pos < 30) {
        tft.setTextColor(C_GOLD, C_BG);
        tft.drawString("price near DAY LOW (" + String(pos) + "%)", 12, y, 2);
        y += 18;
        tft.setTextColor(C_DIM, C_BG);
        tft.drawString("short needs the low to break", 12, y, 2);
        y += 16;
        tft.drawString("long needs it to hold", 12, y, 2);
    } else {
        tft.setTextColor(C_ACCENT, C_BG);
        tft.drawString("price MID range (" + String(pos) + "%)", 12, y, 2);
        y += 18;
        tft.setTextColor(C_DIM, C_BG);
        tft.drawString("no edge from position alone", 12, y, 2);
    }

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("tap to change screen", 160, 232, 2);
    tft.setTextDatum(TL_DATUM);
}

void refreshPrices() {
    drawStatus("updating...", C_ACCENT);
    bool ok = false;
    if (fetchCoinGecko())    { sourceLabel = "CoinGecko"; ok = true; }
    else if (fetchBinance()) { sourceLabel = "Binance";   ok = true; }
    else                     { sourceLabel = "offline"; }
#if SLH_ENABLED
    if (fetchSLH() && ok) sourceLabel += " + SLH";
#endif
    if (ok) { fetchStats(); lastSuccessMs = millis(); }
    static bool sparkOnce = false;
    if (!sparkOnce && ok) { fetchSparklines(); sparkOnce = true; }
    lastRefresh = millis();
    drawCurrentScreen();
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[SLH] Crypto Ticker booting");
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255);
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    tft.setTouch(touchCal);
    rtBegin();

    // hold a finger on the screen during boot to recalibrate
    {
        uint16_t _x, _y;
        if (tft.getTouch(&_x, &_y)) calibrateTouch();
    }
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
    g_lastActivity = millis();
    setupServer();
    refreshPrices();
}

void loop() {
    server.handleClient();
    uint16_t tx, ty;
    {
        uint16_t tx, ty;
        bool touchDown = rtTouch(&tx, &ty);
        bool touchEdge = touchDown && !g_touchWasDown;
        g_touchWasDown = touchDown;

        if (touchEdge && (millis() - g_lastTouchMs) > TOUCH_COOLDOWN_MS) {
            g_lastTouchMs = millis();
            g_lastActivity = millis();
            g_saverActive = false;
            ledcWrite(0, 255);

            if (tx < 52 && ty < 36) {
                if (g_currentScreen != SCR_HOME) {
                    g_currentScreen = SCR_HOME;
                    drawCurrentScreen();
                }
            } else if (g_currentScreen == SCR_HOME) {
                ScreenId tgt = homeTouchHandler(tx, ty);
                if (tgt < SCR_COUNT && tgt != g_currentScreen) {
                    g_currentScreen = tgt;
                    if (tgt == SCR_PRICES) firstDraw = true;
                    if (tgt == SCR_LESSON) g_lessonPage = 0;
                    drawCurrentScreen();
                }
            } else {
                nextScreen();
            }
        }
    }
    if (g_saverOn) {
        unsigned long idle = millis() - g_lastActivity;
        if (!g_saverActive && idle >= SAVER_IDLE_MS) {
            g_saverActive = true;
            g_saverLastFlip = millis();
            ledcWrite(0, 80);
        }
        if (g_saverActive) {
            if (millis() - g_saverLastFlip >= SAVER_CYCLE_MS) {
                g_saverLastFlip = millis();
                nextScreen();
            }
            if (g_currentScreen == SCR_SHOW) updateShowScreen();
            delay(30);
            return;
        }
    }
    if (g_currentScreen == SCR_SHOW) {
        updateShowScreen();
        delay(5);
        return;
    }
    if (g_currentScreen == SCR_LESSON) {
        updateLessonScreen();
        delay(50);
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













