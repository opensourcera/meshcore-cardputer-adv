"""Render a preview of the Signal tester result page at 12 and 18 px (a drawing, not a photo of the device)."""
import os
from PIL import Image, ImageDraw, ImageFont
import matplotlib
TTF = os.path.join(os.path.dirname(matplotlib.__file__), 'mpl-data', 'fonts', 'ttf', 'DejaVuSans.ttf')
W, H, SCALE = 240, 135, 2
GREEN, YELLOW, RED, ORANGE, GRAY, WHITE, BLACK = (0, 255, 0), (255, 255, 0), (255, 0, 0), (255, 165, 0), (128, 128, 128), (255, 255, 255), (0, 0, 0)
TRIES = [(True, 9.5, 11.0), (False, 0, 0), (True, -3.5, -5.2), (True, 7.0, 8.2), (None, 0, 0)]

def ui_lh(px): return 12 if px == 12 else px + 1
def ell(d, f, text, maxw):
    while d.textlength(text, font=f) > maxw and len(text) > 1: text = text[:-1]
    return text
def screen(px, title):
    f = ImageFont.truetype(TTF, px); img = Image.new('RGB', (W, H), BLACK); d = ImageDraw.Draw(img); lh = ui_lh(px)
    tw = d.textlength(title, font=f); d.text(((W - tw) / 2, 5), title, font=f, fill=GREEN)
    top = lh + 8; rows = (H - top) // lh
    first = max(0, len(TRIES) - (rows - 1)); y = top
    for i in range(first, len(TRIES)):
        ok, there, back = TRIES[i]
        d.text((5, y), '#%d' % (i + 1), font=f, fill=YELLOW)
        if ok: d.text((40, y), ell(d, f, '%.1f / %.1f dB' % (there, back), W - 42), font=f, fill=GREEN if there > 0 and back > 0 else ORANGE)
        elif ok is None: d.text((40, y), 'waiting...', font=f, fill=GRAY)
        else: d.text((40, y), 'no reply', font=f, fill=RED)
        y += lh
    d.text((5, top + (rows - 1) * lh), ell(d, f, 'Try 5/5  Esc=stop', W - 7), font=f, fill=GREEN)
    return img.resize((W * SCALE, H * SCALE), Image.NEAREST)

lab = ImageFont.truetype(TTF, 18); gap = 24; sizes = [12, 18]
out = Image.new('RGB', (W * SCALE * 2 + gap * 3, H * SCALE + 40 + gap * 2), (24, 24, 24)); d = ImageDraw.Draw(out)
for i, px in enumerate(sizes):
    cx, cy = gap + i * (W * SCALE + gap), gap
    d.text((cx, cy), 'Signal tester, font size %d' % px, font=lab, fill=WHITE)
    s = screen(px, 'Sweat Mtn via V4'); out.paste(s, (cx, cy + 32)); d.rectangle([cx - 1, cy + 31, cx + s.width, cy + 32 + s.height], outline=(90, 90, 90))
out.save('images/signal_tester_preview.png'); print('wrote images/signal_tester_preview.png', out.size)
