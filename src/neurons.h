#pragma once
// Neural network boot animation - also the base for the tamagotchi eyes

#define NN_NODES 16

struct NNode { float x, y, vx, vy; uint8_t pulse; };
static NNode g_nn[NN_NODES];
static bool  g_nnInit = false;

inline void nnSeed() {
  for (int i = 0; i < NN_NODES; i++) {
    g_nn[i].x = random(20, 300);
    g_nn[i].y = random(20, 150);
    g_nn[i].vx = (random(10, 45) / 100.0f) * (random(2) ? 1 : -1);
    g_nn[i].vy = (random(10, 45) / 100.0f) * (random(2) ? 1 : -1);
    g_nn[i].pulse = random(255);
  }
  g_nnInit = true;
}

inline void nnStep(TFT_eSprite& s, uint16_t accent, uint16_t dim) {
  if (!g_nnInit) nnSeed();
  s.fillSprite(TFT_BLACK);
  for (int i = 0; i < NN_NODES; i++) {
    g_nn[i].x += g_nn[i].vx;
    g_nn[i].y += g_nn[i].vy;
    if (g_nn[i].x < 6 || g_nn[i].x > 314) g_nn[i].vx = -g_nn[i].vx;
    if (g_nn[i].y < 6 || g_nn[i].y > 164) g_nn[i].vy = -g_nn[i].vy;
    g_nn[i].pulse += 3;
  }
  for (int i = 0; i < NN_NODES; i++) {
    for (int j = i + 1; j < NN_NODES; j++) {
      float dx = g_nn[i].x - g_nn[j].x;
      float dy = g_nn[i].y - g_nn[j].y;
      float d2 = dx * dx + dy * dy;
      if (d2 < 7000) s.drawLine(g_nn[i].x, g_nn[i].y, g_nn[j].x, g_nn[j].y, dim);
    }
  }
  for (int i = 0; i < NN_NODES; i++) {
    int r = 2 + (g_nn[i].pulse > 128 ? 1 : 0);
    s.fillCircle(g_nn[i].x, g_nn[i].y, r, accent);
  }
}
