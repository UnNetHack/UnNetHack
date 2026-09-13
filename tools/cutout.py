#!/usr/bin/env python3
"""cutout.py -- remove a flat/gradient background from rendered art,
leaving a transparent PNG ready for tools/mkanim.py.

Usage:
    tools/.venv/bin/python tools/cutout.py <image.png> [more.png ...]
    tools/.venv/bin/python tools/cutout.py frames/            # whole dir

Writes <name>-cut.png next to each input (skips *-cut.png inputs).

Works by flood-filling inward from the image border: a neighbouring
pixel joins the background only if its colour is close to the pixel it
spread from. That tracks smooth gradient backdrops (like a studio
render's vignette) while stopping at the subject's edge, and -- because
it only removes background connected to the border -- dark areas
*inside* the subject (black gloves, shadowed armour) are never eaten.
"""
import sys, os, glob
from collections import deque
from PIL import Image, ImageFilter

# Tuned for studio renders on a neutral vignette backdrop. The subject
# may be *darker* than the backdrop (shadowed leather, dark cloth), so
# brightness alone can't separate them -- but such darks are warm/cool
# tinted while the backdrop stays strictly neutral, so saturation does.
# Override per-image with env vars if a render needs it.
LOCAL_TOL = int(os.environ.get("CUT_LOCAL_TOL", 7))
MAX_LUMA  = int(os.environ.get("CUT_MAX_LUMA", 66))
MAX_SAT   = int(os.environ.get("CUT_MAX_SAT", 9))

def cut(path):
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    px = im.load()
    bg = bytearray(w * h)          # 1 = background
    seen = bytearray(w * h)
    q = deque()

    def luma(c):
        return (c[0] * 299 + c[1] * 587 + c[2] * 114) // 1000

    def sat(c):
        return max(c[:3]) - min(c[:3])

    def push(x, y):
        i = y * w + x
        if seen[i]:
            return
        c = px[x, y]
        if luma(c) <= MAX_LUMA and sat(c) <= MAX_SAT:
            seen[i] = 1
            bg[i] = 1
            q.append((x, y, c))

    for x in range(w):
        push(x, 0); push(x, h - 1)
    for y in range(h):
        push(0, y); push(w - 1, y)

    while q:
        x, y, c = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < w and 0 <= ny < h):
                continue
            i = ny * w + nx
            if seen[i]:
                continue
            n = px[nx, ny]
            if (abs(n[0] - c[0]) <= LOCAL_TOL and abs(n[1] - c[1]) <= LOCAL_TOL
                    and abs(n[2] - c[2]) <= LOCAL_TOL
                    and luma(n) <= MAX_LUMA and sat(n) <= MAX_SAT):
                seen[i] = 1
                bg[i] = 1
                q.append((nx, ny, n))

    alpha = Image.frombytes("L", (w, h),
                            bytes(0 if b else 255 for b in bg))
    # soften the 1px stair-stepping the hard fill leaves behind
    alpha = alpha.filter(ImageFilter.GaussianBlur(0.6))
    im.putalpha(alpha)
    # deliberately NOT cropped here: for an animation sequence every
    # frame must keep the same canvas, or each would be trimmed to its
    # own bounds and the subject would jitter between frames. mkanim.py
    # trims once, using the union of all frames.
    out = os.path.splitext(path)[0] + "-cut.png"
    im.save(out)
    kept = sum(1 for b in bg if not b)
    print(f"{os.path.basename(path)} -> {os.path.basename(out)}  "
          f"{im.width}x{im.height}, {100*kept//(w*h)}% kept")
    return out

def main():
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    paths = []
    for a in args:
        paths += sorted(glob.glob(os.path.join(a, "*.png"))) \
                 if os.path.isdir(a) else [a]
    for p in paths:
        if p.endswith("-cut.png"):
            continue
        cut(p)

if __name__ == "__main__":
    main()
