"""Render a preview of the chat page at each selectable font size (a drawing, not a photo of the device)."""
import os
from PIL import Image, ImageDraw, ImageFont
import matplotlib
TTF = os.path.join(os.path.dirname(matplotlib.__file__), 'mpl-data', 'fonts', 'ttf', 'DejaVuSans.ttf')
W, H, SCALE = 240, 135, 2
GREEN, YELLOW, WHITE, GRAY, BLACK = (0, 255, 0), (255, 255, 0), (255, 255, 255), (128, 128, 128), (0, 0, 0)
MSGS = [(False, 'Anyone around tonight?'), (True, 'Yes, on the roof node'), (False, 'Heard you 2 hops away'), (True, 'Nice, strong signal'), (False, 'See you on #test'), (True, 'ok')]

def ui_lh(px): return 12 if px == 12 else px + 1
def screen(px):
    f = ImageFont.truetype(TTF, px); asc, desc = f.getmetrics(); yadv = asc + desc + 1
    img = Image.new('RGB', (W, H), BLACK); d = ImageDraw.Draw(img); lh = ui_lh(px)
    # status bar: battery icon
    d.rectangle([W - 31, 2, W - 5, 16], outline=GREEN); d.rectangle([W - 29, 4, W - 12, 14], fill=GREEN); d.rectangle([W - 5, 5, W - 2, 12], fill=GREEN)
    y = lh + 4; x = W // 2 - 5 * 6
    for i in range(7):
        if i == 3: d.rectangle([x - 1, y - 1, x + 1, y + 1], fill=GREEN)
        else: d.point((x, y), fill=GREEN)
        x += 10
    top = lh + 8; title = '#test'; tw = d.textlength(title, font=f); d.text(((W - tw) / 2, top), title, font=f, fill=GREEN)
    hist_top = top + lh - 2 + 5; rows = (H - lh - 4 - hist_top) // yadv
    cy = hist_top + (rows - 1) * yadv
    for out, text in reversed(MSGS[-rows:] if rows else []):
        tw = d.textlength(text, font=f)
        d.text((W - tw - 5 if out else 5, cy), text, font=f, fill=GREEN); cy -= yadv
    d.line([1, H - lh - 4, W - 1, H - lh - 4], fill=GREEN)
    d.text((5, H - lh - 4), 'Write a message...', font=f, fill=GRAY)
    return img.resize((W * SCALE, H * SCALE), Image.NEAREST)

sizes = [12, 14, 16, 18]; gap = 24; lab = ImageFont.truetype(TTF, 18)
out = Image.new('RGB', (W * SCALE * 2 + gap * 3, (H * SCALE + 40) * 2 + gap * 3), (24, 24, 24)); d = ImageDraw.Draw(out)
for i, px in enumerate(sizes):
    cx, cy = gap + (i % 2) * (W * SCALE + gap), gap + (i // 2) * (H * SCALE + 40 + gap)
    label = 'Font size: %d' % px + ('  (original)' if px == 12 else '')
    d.text((cx, cy), label, font=lab, fill=WHITE)
    s = screen(px); out.paste(s, (cx, cy + 32)); d.rectangle([cx - 1, cy + 31, cx + s.width, cy + 32 + s.height], outline=(90, 90, 90))
out.save('images/font_sizes_preview.png'); print('wrote images/font_sizes_preview.png', out.size)
