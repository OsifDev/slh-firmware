import os
from PIL import Image, ImageDraw, ImageFont
from bidi.algorithm import get_display

SRC   = r"C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware\src"
FONTS = r"C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware\fonts"
TTF   = os.path.join(FONTS, "DavidLibre-Regular.ttf")

W, H = 320, 200
WHITE=(255,255,255); GOLD=(254,220,140); RED=(249,100,100)
GREEN=(100,240,120); BLUE=(120,200,255); DIM=(150,150,150); BG=(0,0,0)

# category header — 0=fundamentals, 1=calculations, 2=planning
lessons = [

  # ======== FUNDAMENTALS ========
  (0, "\u05e1\u05e4\u05e8\u05d3 - \u05de\u05d4 \u05d6\u05d4?", [
    ('dot',  "\u05d4\u05e4\u05e8\u05e9 \u05d1\u05d9\u05df \u05de\u05d7\u05d9\u05e8 \u05e7\u05e0\u05d9\u05d9\u05d4 \u05dc\u05de\u05db\u05d9\u05e8\u05d4", WHITE),
    ('dot',  "Bid = \u05de\u05d7\u05d9\u05e8 \u05e9\u05e7\u05d5\u05e0\u05d4 \u05de\u05e9\u05dc\u05dd", BLUE),
    ('dot',  "Ask = \u05de\u05d7\u05d9\u05e8 \u05e9\u05de\u05d5\u05db\u05e8 \u05de\u05e7\u05d1\u05dc", BLUE),
    ('dot',  "Spread = Ask - Bid", GOLD),
    ('none', "\u05d6\u05d4 \u05e8\u05d5\u05d5\u05d7 \u05d4\u05d1\u05e8\u05d5\u05e7\u05e8 - \u05e2\u05dc\u05d5\u05ea \u05e1\u05de\u05d5\u05d9\u05d4", DIM),
    ('none', "\u05e7\u05d5\u05e0\u05d4 \u05d5\u05de\u05d5\u05db\u05e8 \u05d1\u05d0\u05d5\u05ea\u05d4 \u05e9\u05e0\u05d9\u05d4 = \u05de\u05e4\u05e1\u05d9\u05d3 \u05d0\u05ea \u05d4\u05e1\u05e4\u05e8\u05d3", RED),
  ]),

  (0, "\u05d3\u05d5\u05d2\u05de\u05d4 - \u05d9\u05d5\u05e8\u05d5 vs \u05d1\u05d9\u05d8\u05e7\u05d5\u05d9\u05df", [
    ('dot',  "EUR/USD \u05d1\u05e4\u05d5\u05e8\u05e7\u05e1", WHITE),
    ('none', "Bid: 1.0850", BLUE),
    ('none', "Ask: 1.0852", BLUE),
    ('none', "\u05e1\u05e4\u05e8\u05d3 = 0.0002 = 2 \u05e4\u05d9\u05e4\u05e1", GOLD),
    ('dot',  "\u05d1\u05d9\u05d8\u05e7\u05d5\u05d9\u05df \u05d1\u05d1\u05d5\u05e8\u05e1\u05d4", WHITE),
    ('none', "Bid: 70,000$  Ask: 70,010$", BLUE),
    ('none', "\u05e1\u05e4\u05e8\u05d3 = 10$ \u05dc\u05d1\u05d9\u05d8\u05e7\u05d5\u05d9\u05df", GOLD),
  ]),

  (0, "\u05e1\u05e4\u05e8\u05d3 \u05e7\u05d8\u05df vs \u05e8\u05d7\u05d1", [
    ('dot',  "\u05e1\u05e4\u05e8\u05d3 \u05e7\u05d8\u05df = \u05e0\u05d6\u05d9\u05dc\u05d5\u05ea \u05d2\u05d1\u05d5\u05d4\u05d4", GREEN),
    ('none', "\u05d9\u05d5\u05ea\u05e8 \u05e7\u05d5\u05e0\u05d9\u05dd \u05d5\u05de\u05d5\u05db\u05e8\u05d9\u05dd", DIM),
    ('none', "\u05e2\u05dc\u05d5\u05ea \u05de\u05e1\u05d7\u05e8 \u05e0\u05de\u05d5\u05db\u05d4", DIM),
    ('dot',  "\u05e1\u05e4\u05e8\u05d3 \u05e8\u05d7\u05d1 = \u05e0\u05d6\u05d9\u05dc\u05d5\u05ea \u05e0\u05de\u05d5\u05db\u05d4", RED),
    ('none', "\u05de\u05d7\u05d9\u05e8 \u05dc\u05d0 \u05d9\u05e6\u05d9\u05d1", DIM),
    ('none', "\u05e2\u05dc\u05d5\u05ea \u05de\u05e1\u05d7\u05e8 \u05d2\u05d1\u05d5\u05d4\u05d4", DIM),
    ('none', "\u05db\u05dc\u05dc: \u05dc\u05e1\u05d7\u05d5\u05e8 \u05e8\u05e7 \u05db\u05e9\u05d4\u05e1\u05e4\u05e8\u05d3 \u05e7\u05d8\u05df", GOLD),
  ]),

  # ======== CALCULATIONS ========
  (1, "\u05d7\u05d9\u05e9\u05d5\u05d1 \u05e2\u05dc\u05d5\u05ea \u05e2\u05e1\u05e7\u05d4", [
    ('dot',  "\u05e2\u05de\u05dc\u05d4 + \u05e1\u05e4\u05e8\u05d3 = \u05e2\u05dc\u05d5\u05ea \u05db\u05dc\u05dc\u05d9\u05ea", WHITE),
    ('dot',  "\u05d1\u05d9\u05e0\u05d0\u05e0\u05e1: 0.1% \u05dc\u05db\u05dc \u05e6\u05d3", WHITE),
    ('none', "\u05e2\u05e1\u05e7\u05d4 1,000$:", BLUE),
    ('none', "\u05db\u05e0\u05d9\u05e1\u05d4: 0.1% = 1$", DIM),
    ('none', "\u05d9\u05e6\u05d9\u05d0\u05d4: 0.1% = 1$", DIM),
    ('none', "\u05e1\u05e4\u05e8\u05d3: 0.05% = 0.5$", DIM),
    ('none', "\u05e1\u05d4\"\u05db \u05e2\u05dc\u05d5\u05ea: 2.5$ \u05dc\u05db\u05dc \u05e2\u05e1\u05e7\u05d4", GOLD),
  ]),

  (1, "\u05e2\u05dc\u05d5\u05ea vs \u05ea\u05e0\u05d5\u05e2\u05d4 - \u05d9\u05d7\u05e1 \u05d7\u05e9\u05d5\u05d1", [
    ('dot',  "\u05d0\u05dd \u05d4\u05e2\u05dc\u05d5\u05ea > 30% \u05de\u05d4\u05ea\u05e0\u05d5\u05e2\u05d4 - \u05d0\u05dc \u05ea\u05e1\u05d7\u05d5\u05e8", WHITE),
    ('dot',  "\u05d1\u05d9\u05d8\u05e7\u05d5\u05d9\u05df \u05d6\u05d6 5% \u05d1\u05d9\u05d5\u05dd = \u05d8\u05d5\u05d1", GREEN),
    ('dot',  "\u05d0\u05ea\u05e8\u05d9\u05d5\u05dd \u05d6\u05d6 9% \u05d1\u05d9\u05d5\u05dd = \u05de\u05e2\u05d5\u05dc\u05d4", GREEN),
    ('none', "\u05d7\u05d9\u05e9\u05d5\u05d1:", BLUE),
    ('none', "\u05e2\u05dc\u05d5\u05ea 2.5$ \u05dc\u05e2\u05e1\u05e7\u05d4 1000$", DIM),
    ('none', "\u05e6\u05e8\u05d9\u05da \u05dc\u05e0\u05e6\u05d5\u05d7 0.25% \u05e8\u05e7 \u05dc\u05e9\u05d1\u05d5\u05e8 \u05e9\u05d5\u05d5\u05d4", GOLD),
  ]),

  (1, "\u05d7\u05d9\u05e9\u05d5\u05d1 \u05e1\u05d8\u05d5\u05e4 \u05dc\u05d5\u05e1", [
    ('dot',  "\u05e1\u05d8\u05d5\u05e4 \u05dc\u05d5\u05e1 = \u05de\u05d7\u05d9\u05e8 \u05e9\u05d4\u05e2\u05e1\u05e7\u05d4 \u05e0\u05e1\u05d2\u05e8\u05ea \u05d1\u05d4\u05e4\u05e1\u05d3", WHITE),
    ('dot',  "\u05de\u05d7\u05e9\u05d1\u05d9\u05dd \u05dc\u05e4\u05e0\u05d9 \u05e9\u05e0\u05db\u05e0\u05e1\u05d9\u05dd", GOLD),
    ('none', "\u05d7\u05e9\u05d1\u05d5\u05df 10,000$", BLUE),
    ('none', "\u05e1\u05d9\u05db\u05d5\u05df \u05de\u05e7\u05e1\u05d9\u05de\u05dc\u05d9 1% = 100$", DIM),
    ('none', "\u05e1\u05d8\u05d5\u05e4 1% \u05de\u05ea\u05d7\u05ea = \u05e4\u05d5\u05d6\u05d9\u05e6\u05d9\u05d4 10,000$", GREEN),
    ('none', "\u05e1\u05d8\u05d5\u05e4 0.5% = \u05e4\u05d5\u05d6\u05d9\u05e6\u05d9\u05d4 20,000$", GREEN),
  ]),

  # ======== PLANNING ========
  (2, "\u05ea\u05db\u05e0\u05d5\u05df \u05e2\u05e1\u05e7\u05d4 - 6 \u05e9\u05dc\u05d1\u05d9\u05dd", [
    ('dot',  "1. \u05d1\u05d7\u05e8 \u05de\u05d8\u05d1\u05e2 \u05e2\u05dd \u05ea\u05e0\u05d5\u05e2\u05d4 \u05de\u05e2\u05dc 3%", WHITE),
    ('dot',  "2. \u05e7\u05d1\u05e2 \u05de\u05d7\u05d9\u05e8 \u05db\u05e0\u05d9\u05e1\u05d4 \u05de\u05d3\u05d5\u05d9\u05e7", WHITE),
    ('dot',  "3. \u05e7\u05d1\u05e2 \u05e1\u05d8\u05d5\u05e4 \u05dc\u05d5\u05e1 1% \u05de\u05ea\u05d7\u05ea", WHITE),
    ('dot',  "4. \u05e7\u05d1\u05e2 \u05d9\u05e2\u05d3 2% \u05de\u05e2\u05dc", WHITE),
    ('dot',  "5. \u05d7\u05e9\u05d1 \u05d2\u05d5\u05d3\u05dc \u05e4\u05d5\u05d6\u05d9\u05e6\u05d9\u05d4", WHITE),
    ('none', "6. \u05e8\u05e9\u05d5\u05dd \u05d1\u05d9\u05d5\u05de\u05df \u05dc\u05e4\u05e0\u05d9 \u05d1\u05d9\u05e6\u05d5\u05e2", GOLD),
  ]),

  (2, "\u05d9\u05d7\u05e1 \u05e1\u05d9\u05db\u05d5\u05df-\u05e1\u05d9\u05db\u05d5\u05d9 (R:R)", [
    ('dot',  "\u05db\u05de\u05d4 \u05ea\u05e1\u05d9\u05db\u05df \u05de\u05d5\u05dc \u05db\u05de\u05d4 \u05ea\u05e8\u05d5\u05d5\u05d9\u05d7", WHITE),
    ('dot',  "\u05de\u05d9\u05e0\u05d9\u05de\u05d5\u05dd 1:2 \u05d1\u05db\u05dc \u05e2\u05e1\u05e7\u05d4", GOLD),
    ('none', "10 \u05e2\u05e1\u05e7\u05d0\u05d5\u05ea \u05e2\u05dd \u05e1\u05d9\u05db\u05d5\u05df 100$ \u05d5\u05d9\u05e2\u05d3 200$", BLUE),
    ('none', "4 \u05e0\u05e6\u05d7\u05d5\u05ea: +800$", GREEN),
    ('none', "6 \u05d4\u05e4\u05e1\u05d3\u05d5\u05ea: -600$", RED),
    ('none', "\u05e1\u05d4\"\u05db: +200$ \u05e8\u05d5\u05d5\u05d7 \u05e0\u05e7\u05d9", GOLD),
    ('none', "\u05d2\u05dd 40% \u05d4\u05e6\u05dc\u05d7\u05d4 = \u05e8\u05d5\u05d5\u05d7", GREEN),
  ]),

  (2, "\u05d0\u05d9\u05da \u05dc\u05d4\u05d9\u05db\u05e0\u05e1 \u05dc\u05e2\u05e1\u05e7\u05d4", [
    ('dot',  "\u05d4\u05de\u05ea\u05df \u05dc\u05de\u05d7\u05d9\u05e8 \u05d1\u05e7\u05e6\u05d4 \u05d4\u05d8\u05d5\u05d5\u05d7", WHITE),
    ('dot',  "\u05dc\u05d0 \u05dc\u05e8\u05d3\u05d5\u05e3 \u05d0\u05d7\u05e8\u05d9 \u05de\u05d7\u05d9\u05e8 \u05e9\u05e2\u05d5\u05dc\u05d4", RED),
    ('dot',  "\u05db\u05e0\u05d9\u05e1\u05d4 \u05d1\u05e1\u05d8\u05d5\u05e4 \u05dc\u05d5\u05e1, \u05dc\u05d0 \u05d1\u05e4\u05d0\u05de\u05e4", WHITE),
    ('dot',  "\u05e4\u05d9\u05e1\u05e4\u05e1\u05ea \u05d4\u05db\u05e0\u05d9\u05e1\u05d4? \u05d9\u05e9 \u05e2\u05d5\u05d3 100 \u05e2\u05e1\u05e7\u05d0\u05d5\u05ea", DIM),
    ('none', "FOMO = \u05e4\u05d7\u05d3 \u05dc\u05d4\u05e4\u05e1\u05d9\u05d3 = \u05e8\u05e9\u05d9\u05de\u05ea \u05d4\u05e8\u05d5\u05d5\u05d7\u05d4", GOLD),
  ]),

  (2, "\u05d8\u05e2\u05d5\u05d9\u05d5\u05ea \u05e9\u05d4\u05d5\u05e8\u05d2\u05d5\u05ea \u05d7\u05e9\u05d1\u05d5\u05e0\u05d5\u05ea", [
    ('dot',  "\u05dc\u05d4\u05d6\u05d9\u05d6 \u05e1\u05d8\u05d5\u05e4 \u05db\u05e9\u05d4\u05de\u05d7\u05d9\u05e8 \u05d6\u05d6 \u05e0\u05d2\u05d3\u05da", RED),
    ('dot',  "\u05dc\u05d4\u05d2\u05d3\u05d9\u05dc \u05e4\u05d5\u05d6\u05d9\u05e6\u05d9\u05d4 \u05d0\u05d7\u05e8\u05d9 \u05d4\u05e4\u05e1\u05d3", RED),
    ('dot',  "\u05dc\u05e1\u05d7\u05d5\u05e8 \u05d1\u05dc\u05d9 \u05ea\u05d5\u05db\u05e0\u05d9\u05ea", RED),
    ('dot',  "\u05dc\u05e1\u05d7\u05d5\u05e8 \u05de\u05ea\u05d5\u05da \u05db\u05e2\u05e1", RED),
    ('dot',  "\u05dc\u05d0 \u05dc\u05ea\u05e2\u05d3 \u05e2\u05e1\u05e7\u05d0\u05d5\u05ea", DIM),
    ('none', "90% \u05de\u05d4\u05e1\u05d5\u05d7\u05e8\u05d9\u05dd \u05de\u05e4\u05e1\u05d9\u05d3\u05d9\u05dd \u05d1\u05d2\u05dc\u05dc \u05d0\u05dc\u05d4", GOLD),
  ]),

  (2, "\u05d4\u05d0\u05de\u05ea \u05e2\u05dc \u05e8\u05d5\u05d5\u05d7\u05d9\u05dd", [
    ('dot',  "\u05dc\u05d0 \u05d4\u05d9\u05de\u05d5\u05e8 \u05d2\u05d3\u05d5\u05dc - \u05d0\u05dc\u05d0 100 \u05e7\u05d8\u05e0\u05d9\u05dd", WHITE),
    ('dot',  "\u05d7\u05d5\u05e7\u05d9\u05dd \u05d1\u05e8\u05d5\u05e8\u05d9\u05dd \u05dc\u05db\u05dc \u05e2\u05e1\u05e7\u05d4", WHITE),
    ('dot',  "\u05e1\u05d9\u05db\u05d5\u05df \u05e7\u05d8\u05df \u05d1\u05db\u05dc \u05e4\u05e2\u05dd", WHITE),
    ('dot',  "\u05d4\u05de\u05ea\u05df \u05dc\u05ea\u05d5\u05e6\u05d0\u05d4 \u05d4\u05d7\u05d9\u05d5\u05d1\u05d9\u05ea", WHITE),
    ('none', "\u05d6\u05d4 \u05de\u05e9\u05e2\u05de\u05dd - \u05d5\u05d6\u05d4 \u05d1\u05d3\u05d9\u05d5\u05e7 \u05dc\u05de\u05d4 \u05e9\u05d4\u05d5\u05d0 \u05e2\u05d5\u05d1\u05d3", GOLD),
  ]),
]

CAT_NAMES = [
  ("\u05d9\u05e1\u05d5\u05d3\u05d5\u05ea", BLUE),
  ("\u05d7\u05d9\u05e9\u05d5\u05d1\u05d9\u05dd", GOLD),
  ("\u05ea\u05db\u05e0\u05d5\u05df", GREEN),
]

title_font = ImageFont.truetype(TTF, 26)
line_font  = ImageFont.truetype(TTF, 17)
cat_font   = ImageFont.truetype(TTF, 13)

pages = []
for idx, (cat, title, lines) in enumerate(lessons):
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, W, 34], fill=(20, 20, 30))
    d.line([0, 34, W, 34], fill=GOLD, width=2)
    vt = get_display(title)
    bbox = d.textbbox((0, 0), vt, font=title_font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    d.text(((W - tw) // 2 - bbox[0], (34 - th) // 2 - bbox[1]), vt, font=title_font, fill=GOLD)
    cat_name, cat_color = CAT_NAMES[cat]
    vc = get_display(cat_name)
    d.rectangle([0, H - 18, W, H], fill=(20, 20, 30))
    d.text((8, H - 15), vc, font=cat_font, fill=cat_color)
    page_str = "%d/%d" % (idx + 1, len(lessons))
    d.text((W - 40, H - 15), page_str, font=cat_font, fill=DIM)
    y = 46
    for btype, text, color in lines:
        v = get_display(text)
        bbox = d.textbbox((0, 0), v, font=line_font)
        tw = bbox[2] - bbox[0]
        if btype == 'dot':
            d.ellipse([W - 12, y + 7, W - 6, y + 13], fill=color)
            text_right = W - 20
        else:
            text_right = W - 12
        d.text((text_right - tw - bbox[0], y), v, font=line_font, fill=color)
        y += 22
    png = os.path.join(FONTS, "lesson_%d.png" % idx)
    img.save(png)
    px = img.load()
    rgb = bytearray(W * H)
    for yy in range(H):
        for xx in range(W):
            r, g, b = px[xx, yy]
            rgb[yy * W + xx] = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)
    pages.append(rgb)
    print("[OK] page %d" % idx)

out = os.path.join(SRC, "lesson_bitmaps.h")
with open(out, "w", encoding="ascii") as f:
    f.write("// AUTO-GENERATED by render_lessons.py\n#pragma once\n#include <Arduino.h>\n\n")
    f.write("const uint16_t LESSON_W   = %d;\n" % W)
    f.write("const uint16_t LESSON_H   = %d;\n" % H)
    f.write("const uint16_t LESSON_TOP = 0;\n")
    f.write("const uint8_t  LESSON_COUNT = %d;\n\n" % len(pages))
    for i, data in enumerate(pages):
        f.write("const uint8_t LESSON_PAGE_%d[%d] PROGMEM = {\n" % (i, len(data)))
        for j in range(0, len(data), 16):
            f.write("  " + ",".join("0x%02X" % b for b in data[j:j+16]) + ",\n")
        f.write("};\n\n")
    f.write("const uint8_t* const LESSON_PAGES[%d] = {" % len(pages))
    f.write(", ".join("LESSON_PAGE_%d" % i for i in range(len(pages))))
    f.write("};\n")
print("done12")