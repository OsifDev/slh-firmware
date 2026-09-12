#pragma once
// Draw tracker - records what was painted so /render can replay it

#define DT_MAX 48
struct DrawOp { int16_t x, y, w, h; uint16_t col; char txt[20]; uint8_t kind; };
DrawOp g_dt[DT_MAX];
int g_dtN = 0;
bool g_dtOn = true;

inline void dtReset() { g_dtN = 0; }

inline void dtText(int x, int y, const String& s, uint16_t c) {
  if (!g_dtOn || g_dtN >= DT_MAX) return;
  DrawOp& o = g_dt[g_dtN++];
  o.kind = 1; o.x = x; o.y = y; o.w = 0; o.h = 0; o.col = c;
  s.toCharArray(o.txt, 20);
}

inline void dtRect(int x, int y, int w, int h, uint16_t c) {
  if (!g_dtOn || g_dtN >= DT_MAX) return;
  DrawOp& o = g_dt[g_dtN++];
  o.kind = 2; o.x = x; o.y = y; o.w = w; o.h = h; o.col = c;
  o.txt[0] = 0;
}
