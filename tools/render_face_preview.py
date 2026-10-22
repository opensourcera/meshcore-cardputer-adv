"""Render the chat page in Sans and Mono at 16 px (a drawing, not a photo of the device)."""
import os
from PIL import Image, ImageDraw, ImageFont
import matplotlib
TTFDIR = os.path.join(os.path.dirname(matplotlib.__file__), 'mpl-data', 'fonts', 'ttf')
W, H, SCALE = 240, 135, 2
GREEN, GRAY, WHITE, BLACK = (0, 255, 0), (128, 128, 128), (255, 255, 255), (0, 0, 0)
MSGS = [(False, 'Anyone around tonight?'), (True, 'Yes, on the roof node'), (False, 'Heard you 2 hops away'), (True, 'Nice, strong signal'), (False, 'See you on #test'), (True, 'ok')]

def screen(ttf, px):
    f = ImageFont.truetype(os.path.join(TTFDIR, ttf), px); asc, desc = f.getmetrics(); yadv = asc + desc + 1
    img = Image.new('RGB', (W, H), BLACK); d = ImageDraw.Draw(img); lh = px + 1
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

lab = ImageFont.truetype(os.path.join(TTFDIR, 'DejaVuSans.ttf'), 18); gap = 24
faces = [('DejaVuSans.ttf', 'Font: Sans, 16 px'), ('DejaVuSansMono.ttf', 'Font: Mono, 16 px')]
out = Image.new('RGB', (W * SCALE * 2 + gap * 3, H * SCALE + 40 + gap * 2), (24, 24, 24)); d = ImageDraw.Draw(out)
for i, (ttf, label) in enumerate(faces):
    cx, cy = gap + i * (W * SCALE + gap), gap
    d.text((cx, cy), label, font=lab, fill=WHITE)
    s = screen(ttf, 16); out.paste(s, (cx, cy + 32)); d.rectangle([cx - 1, cy + 31, cx + s.width, cy + 32 + s.height], outline=(90, 90, 90))
out.save('images/font_faces_preview.png'); print('wrote images/font_faces_preview.png', out.size)
