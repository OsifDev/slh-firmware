#!/usr/bin/env python3
# render_lessons.py v2 — better typography + clearer content
import os, sys
from PIL import Image, ImageDraw, ImageFont
from bidi.algorithm import get_display

SRC   = r"C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware\src"
FONTS = r"C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware\fonts"
TTF   = os.path.join(FONTS, "DavidLibre-Regular.ttf")

W, H = 320, 172

WHITE = (255,255,255)
GOLD  = (254,220,140)
RED   = (249,100,100)
GREEN = (100,240,120)
BLUE  = (120,200,255)
DIM   = (150,150,150)
BG    = (0,0,0)

# ================== LESSONS ==================
# Structure: (title, [ (bullet_type, text, color) ])
#   bullet_type: 'dot' = filled circle, 'none' = no bullet
# =============================================

lessons = [

  # ------- PAGE 0 : SPREAD -------
  ("\u05de\u05b8\u05d4 \u05d6\u05b6\u05d4 \u05e1\u05b0\u05e4\u05b0\u05bc\u05e8\u05b6\u05d3", [
    ('dot',  "\u05dc\u05b0\u05db\u05b8\u05dc \u05de\u05b7\u05d8\u05b0\u05d1\u05b5\u05bc\u05e2\u05b7 \u05d9\u05b5\u05e9 \u05e9\u05c1\u05b0\u05e0\u05b5\u05d9 \u05de\u05b0\u05d7\u05b4\u05d9\u05e8\u05b4\u05d9\u05dd", WHITE),
    ('dot',  "\u05de\u05b0\u05d7\u05b4\u05d9\u05e8 \u05e7\u05b0\u05e0\u05b4\u05d9\u05b8\u05bc\u05d4 - \u05d2\u05b8\u05bc\u05d1\u05d5\u05b9\u05d4\u05b7", DIM),
    ('dot',  "\u05de\u05b0\u05d7\u05b4\u05d9\u05e8 \u05de\u05b0\u05db\u05b4\u05d9\u05e8\u05b8\u05d4 - \u05e0\u05b8\u05de\u05d5\u05bc\u05da\u05b0", DIM),
    ('dot',  "\u05d4\u05b6\u05e4\u05b0\u05e8\u05b5\u05e9\u05c1 = \u05e1\u05b0\u05e4\u05b0\u05bc\u05e8\u05b6\u05d3", GOLD),
    ('none', "\u05d3\u05bc\u05d5\u05bc\u05d2\u05b0\u05de\u05b8\u05d4: \u05e7\u05d5\u05b9\u05e0\u05b6\u05d4 \u05d1-100, \u05de\u05d5\u05b9\u05db\u05b5\u05e8 \u05d1-99.95", GREEN),
  ]),

  # ------- PAGE 1 : WHEN -------
  ("\u05de\u05b8\u05ea\u05b7\u05d9 \u05dc\u05b0\u05d4\u05b4\u05d9\u05db\u05b8\u05bc\u05e0\u05b5\u05e1", [
    ('dot',  "1. \u05ea\u05b0\u05bc\u05e0\u05d5\u05bc\u05e2\u05b8\u05d4 \u05d9\u05d5\u05b9\u05de\u05b4\u05d9\u05ea \u05d2\u05b0\u05bc\u05d1\u05d5\u05b9\u05d4\u05b8\u05d4 \u05de-3%", WHITE),
    ('none', "    \u05e4\u05b8\u05bc\u05d7\u05d5\u05b9\u05ea \u05de\u05b4\u05d6\u05b6\u05bc\u05d4 - \u05d0\u05b5\u05d9\u05df \u05de\u05b8\u05d4 \u05dc\u05b0\u05d7\u05b7\u05e4\u05b5\u05bc\u05e9\u05c2", DIM),
    ('dot',  "2. \u05e2\u05b2\u05de\u05b8\u05dc\u05b8\u05d4 \u05e4\u05b8\u05bc\u05d7\u05d5\u05b9\u05ea \u05de-30% \u05de\u05b4\u05d4\u05b7\u05ea\u05b0\u05bc\u05e0\u05d5\u05bc\u05e2\u05b8\u05d4", WHITE),
    ('none', "    \u05d0\u05b7\u05d7\u05b5\u05e8\u05ea - \u05d4\u05b8\u05e2\u05b2\u05de\u05b8\u05dc\u05b8\u05d4 \u05d0\u05d5\u05b9\u05db\u05b6\u05dc\u05b6\u05ea \u05d4\u05b7\u05db\u05b9\u05bc\u05dc", DIM),
    ('dot',  "3. \u05de\u05b0\u05d7\u05b4\u05d9\u05e8 \u05d1\u05b7\u05e7\u05b0\u05bc\u05e6\u05b8\u05d5\u05b5\u05d9 \u05d4\u05b7\u05d8\u05b0\u05bc\u05d5\u05b8\u05d7", WHITE),
  ]),

  # ------- PAGE 2 : RISK -------
  ("\u05db\u05b7\u05de\u05b8\u05bc\u05d4 \u05dc\u05b0\u05e1\u05b7\u05db\u05b5\u05bc\u05df?", [
    ('dot',  "10 \u05d4\u05b6\u05e4\u05b0\u05e1\u05b5\u05d3\u05b4\u05d9\u05dd \u05d1\u05b0\u05bc\u05e8\u05b6\u05e6\u05b6\u05e3 - \u05de\u05b8\u05d4 \u05e7\u05d5\u05b9\u05e8\u05b6\u05d4?", WHITE),
    ('dot',  "\u05d1\u05b0\u05bc-1% \u05dc\u05b0\u05e2\u05b4\u05e1\u05b0\u05e7\u05b8\u05d4: 10% \u05de\u05b5\u05d4\u05b7\u05d7\u05b4\u05e9\u05c1\u05b0\u05d1\u05bc\u05d5\u05b9\u05df", GREEN),
    ('dot',  "\u05d1\u05b0\u05bc-10% \u05dc\u05b0\u05e2\u05b4\u05e1\u05b0\u05e7\u05b8\u05d4: \u05db\u05b8\u05bc\u05dc \u05d4\u05b7\u05d7\u05b4\u05e9\u05c1\u05b0\u05d1\u05bc\u05d5\u05b9\u05df", RED),
    ('dot',  "\u05d7\u05d5\u05b9\u05e7: 1% \u05dc\u05b0\u05e2\u05b4\u05e1\u05b0\u05e7\u05b8\u05d4 \u05de\u05b7\u05e7\u05b0\u05e1\u05b4\u05d9\u05de\u05d5\u05bc\u05dd", GOLD),
    ('none', "\u05e1\u05b4\u05d9\u05db\u05bc\u05d5\u05bc\u05df 10 \u05d3\u05bc\u05d5\u05b9\u05dc\u05b8\u05e8 \u05dc\u05b0-1000 \u05d3\u05bc\u05d5\u05b9\u05dc\u05b8\u05e8 \u05e4\u05bc\u05d5\u05b9\u05d6\u05b4\u05d9\u05e6\u05b0\u05d9\u05b8\u05d4", BLUE),
  ]),

  # ------- PAGE 3 : ALGO -------
  ("\u05dc\u05b8\u05de\u05b8\u05d4 \u05d0\u05b7\u05dc\u05b0\u05d2\u05bc\u05d5\u05b9\u05e8\u05b4\u05d9\u05ea\u05b0\u05de\u05b4\u05d9?", [
    ('dot',  "\u05d1\u05b6\u05bc\u05df \u05d0\u05b8\u05d3\u05b8\u05dd \u05de\u05b7\u05d7\u05b0\u05dc\u05b4\u05d9\u05d8 \u05de\u05b5\u05d4\u05b7\u05d1\u05bc\u05b6\u05d8\u05b6\u05df", RED),
    ('dot',  "\u05de\u05b0\u05db\u05d5\u05b9\u05e0\u05b8\u05d4 \u05dc\u05b0\u05e4\u05b4\u05d9 \u05d7\u05d5\u05b9\u05e7\u05b4\u05d9\u05dd", GREEN),
    ('dot',  "\u05ea\u05b7\u05bc\u05e2\u05b7\u05d3 \u05db\u05b8\u05bc\u05dc \u05e2\u05b4\u05e1\u05b0\u05e7\u05b8\u05d4 - \u05ea\u05b4\u05bc\u05dc\u05b0\u05de\u05b7\u05d3", WHITE),
    ('dot',  "\u05d7\u05b2\u05d6\u05d5\u05b9\u05e8 \u05e2\u05b7\u05dc \u05d0\u05d5\u05b9\u05ea\u05b8\u05d4 \u05d4\u05b7\u05d2\u05b0\u05bc\u05d3\u05b8\u05e8\u05b8\u05d4 100 \u05e4\u05bc\u05b0\u05e2\u05b8\u05de\u05b4\u05d9\u05dd", WHITE),
    ('none', "\u05de\u05b0\u05e9\u05c1\u05b7\u05e2\u05b2\u05de\u05b5\u05dd \u05de\u05b0\u05e0\u05b7\u05e6\u05b5\u05bc\u05d7\u05b7.   \u05de\u05b0\u05e8\u05b7\u05d2\u05b5\u05bc\u05e9\u05c1 \u05de\u05b7\u05e4\u05b0\u05e1\u05b4\u05d9\u05d3.", GOLD),
  ]),
]

# ================== RENDER ==================
title_font = ImageFont.truetype(TTF, 28)
line_font  = ImageFont.truetype(TTF, 18)

pages = []
for idx, (title, lines) in enumerate(lessons):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)

    # --- top strip (thin gold line + title) ---
    d.rectangle([0, 0, W, 34], fill=(20, 20, 30))
    d.line([0, 34, W, 34], fill=GOLD, width=2)

    vt = get_display(title)
    bbox = d.textbbox((0, 0), vt, font=title_font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    d.text(((W - tw) // 2 - bbox[0], (34 - th) // 2 - bbox[1]), vt, font=title_font, fill=GOLD)

    # --- body ---
    y = 48
    for btype, text, color in lines:
        v = get_display(text)
        bbox = d.textbbox((0, 0), v, font=line_font)
        tw = bbox[2] - bbox[0]

        # bullet dot (small, colored)
        if btype == 'dot':
            d.ellipse([W - 12, y + 8, W - 6, y + 14], fill=color)
            text_right = W - 20
        else:
            text_right = W - 12

        # right-aligned text
        d.text((text_right - tw - bbox[0], y), v, font=line_font, fill=color)
        y += 24

    png = os.path.join(FONTS, "lesson_%d.png" % idx)
    img.save(png)
    print("[OK] preview:", png)

    # RGB332 quantization
    px = img.load()
    rgb = bytearray(W * H)
    for yy in range(H):
        for xx in range(W):
            r, g, b = px[xx, yy]
            rgb[yy * W + xx] = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)
    pages.append(rgb)
    print("[OK] page %d: %d bytes" % (idx, len(rgb)))

# ================== HEADER ==================
out = os.path.join(SRC, "lesson_bitmaps.h")
with open(out, "w", encoding="ascii") as f:
    f.write("// AUTO-GENERATED by render_lessons.py\n")
    f.write("#pragma once\n#include <Arduino.h>\n\n")
    f.write("const uint16_t LESSON_W   = %d;\n" % W)
    f.write("const uint16_t LESSON_H   = %d;\n" % H)
    f.write("const uint16_t LESSON_TOP = 0;\n\n")
    for i, data in enumerate(pages):
        f.write("const uint8_t LESSON_PAGE_%d[%d] PROGMEM = {\n" % (i, len(data)))
        for j in range(0, len(data), 16):
            f.write("  " + ",".join("0x%02X" % b for b in data[j:j+16]) + ",\n")
        f.write("};\n\n")
    f.write("const uint8_t* const LESSON_PAGES[4] = { LESSON_PAGE_0, LESSON_PAGE_1, LESSON_PAGE_2, LESSON_PAGE_3 };\n")

print("[OK] header:", out, "(%d bytes)" % os.path.getsize(out))