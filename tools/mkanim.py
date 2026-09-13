#!/usr/bin/env python3
"""mkanim.py -- turn exported animation frames into an UnNetHack SDL
animated tile overlay strip.

Usage:
    tools/.venv/bin/python tools/mkanim.py <tile-or-name> frame1.png frame2.png ...
    tools/.venv/bin/python tools/mkanim.py <tile-or-name> frames_dir/

<tile-or-name> is either a tile number, or a name looked up in
tilesets/TILE-INDEX.txt (case-insensitive substring; must match
exactly one entry -- grep the file yourself to disambiguate).

Frames may be any size and don't need to be square: each is
alpha-trimmed, centered on a square transparent canvas, resized to the
first frame's scale, and laid out horizontally. Output goes to
tilesets/anim/<NNNN>.png. Copy or `make install` it into
HACKDIR/anim/ and relaunch the game.

Setup (once):  python3 -m venv tools/.venv
               tools/.venv/bin/pip install pillow
"""
import sys, os, glob
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INDEX = os.path.join(ROOT, "tilesets", "TILE-INDEX.txt")
OUTDIR = os.path.join(ROOT, "tilesets", "anim")

def resolve_tile(arg):
    if arg.isdigit():
        return int(arg)
    hits = []
    with open(INDEX) as f:
        for line in f:
            if line.startswith("#"):
                continue
            num, kind, name = line.rstrip("\n").split("\t")
            if arg.lower() in name.lower():
                hits.append((int(num), kind, name))
    exact = [h for h in hits if h[2].lower() == arg.lower()]
    if len(exact) == 1:
        hits = exact
    if len(hits) == 1:
        print(f"tile {hits[0][0]} = {hits[0][1]} '{hits[0][2]}'")
        return hits[0][0]
    sys.exit(f"'{arg}' matches {len(hits)} entries in TILE-INDEX.txt: "
             + ", ".join(f"{n} ({m})" for n, _, m in hits[:12])
             + (" ..." if len(hits) > 12 else "")
             or f"'{arg}': no match")

def load_frames(args):
    paths = []
    for a in args:
        if os.path.isdir(a):
            paths += sorted(glob.glob(os.path.join(a, "*.png")))
        else:
            paths.append(a)
    if not paths:
        sys.exit("no frames given")
    frames = [Image.open(p).convert("RGBA") for p in paths]
    # Alpha-trim using the UNION of every frame's bounds, not each
    # frame's own -- per-frame trimming re-centres the subject every
    # frame and makes an animation jitter.
    boxes = [f.getbbox() for f in frames if f.getbbox()]
    if boxes:
        u = (min(b[0] for b in boxes), min(b[1] for b in boxes),
             max(b[2] for b in boxes), max(b[3] for b in boxes))
        frames = [f.crop(u) for f in frames]
    return frames

MAX_SIDE = int(os.environ.get("ANIM_SIZE", 256))  # per-frame cap in px

def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    tile = resolve_tile(sys.argv[1])
    frames = load_frames(sys.argv[2:])
    # square cell: fits the largest trimmed frame, with 4% breathing room
    side = max(max(f.width, f.height) for f in frames)
    side = int(side * 1.04)
    if side > MAX_SIDE:
        # tiles render ~32px; anything past a few hundred px is wasted
        # file size. Scale frames down together to preserve alignment.
        scale = MAX_SIDE / side
        frames = [f.resize((max(1, int(f.width * scale)),
                            max(1, int(f.height * scale))), Image.LANCZOS)
                  for f in frames]
        side = MAX_SIDE
    strip = Image.new("RGBA", (side * len(frames), side), (0, 0, 0, 0))
    for i, f in enumerate(frames):
        strip.paste(f, (i * side + (side - f.width) // 2,
                        side - f.height))  # bottom-aligned, centered
    os.makedirs(OUTDIR, exist_ok=True)
    out = os.path.join(OUTDIR, f"{tile:04d}.png")
    strip.save(out)
    print(f"wrote {out}: {len(frames)} frames, {side}x{side} each")

if __name__ == "__main__":
    main()
