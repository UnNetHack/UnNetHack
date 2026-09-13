/* NetHack SDL2 window port */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Scaffold implementation.  It gives every UnNetHack window a real SDL2
 * window and a real event loop.  The map is drawn with real tile art
 * (tilesets/unchozo32b.png, the only bundled sheet with enough tiles to
 * cover this fork's total_tiles_used), via the same glyph2tile[] table
 * (src/tile.c, generated at build time) the X11/Qt ports use -- see
 * win/X11/tile2x11.c for the reference this was modeled on.  If no
 * tileset can be found at runtime, map cells fall back to a single
 * colored character per glyph via mapglyph(), so the port still works
 * without the asset.  See win/sdl/Install.SDL2 for build instructions
 * and the current list of known gaps (no real player_selection dialog,
 * no font hinting/DPI handling, no persistent inventory window, etc).
 */

/*
 * SDL2's headers must be parsed before hack.h: hack.h #defines a
 * yn(query) macro that collides with the yn(int,double) Bessel
 * function declared in <math.h>, which SDL_stdinc.h pulls in
 * transitively. Getting SDL's system-header includes out of the way
 * first avoids the clash; see include/winsdl.h for the other half of
 * this note.
 */
#define SDL_MAIN_HANDLED /* NetHack owns main(); don't let SDL rename it */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "hack.h"
#include "color.h"
#include "dlb.h"
#include "func_tab.h"
#include "winsdl.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define CELL_PAD 2      /* pixels of breathing room around each glyph */
#define MSG_ROWS 2      /* lines reserved for the message window, top */
#define STATUS_ROWS 2   /* lines reserved for the status window, bottom */
#define SDL_TILES_PER_ROW 40 /* fixed layout convention every tile sheet
                                 in tilesets/ and win/X11 shares */
#define VIEW_COLS 50    /* map columns visible at once. COLNO(80) at
                            native tile size is 2560px -- wider than
                            most screens -- so the map viewport scrolls
                            horizontally with the player instead of
                            shrinking everything to fit the full width */

/* glyph -> tile index, generated at build time into src/tile.c from
   win/share/monsters.txt|objects.txt|other.txt by util/tilemap */
extern short glyph2tile[];
extern int total_tiles_used;

sdl_window sdl_windows[SDL_MAXWIN];

static SDL_Window *g_win = NULL;
static SDL_Renderer *g_ren = NULL;
static TTF_Font *g_font = NULL;
static int g_cell_w = 8, g_cell_h = 14; /* text (message/status/menu) cells */

static SDL_Texture *g_tile_tex = NULL;
static int g_tile_w = 32, g_tile_h = 32; /* map cells, once tiles load */

/* Per-tile animation overlays: HACKDIR/anim/NNNN.png, where NNNN is the
   tile number and the image is a horizontal strip of square frames at
   any resolution (frame size = image height); frames get scaled into
   the tile cell, so high-definition art just works.  Tiles without an
   overlay keep their static sheet tile -- art can arrive one piece at
   a time.  All instances of a tile animate in sync off the wall clock,
   decoupled from game turns. */
typedef struct {
    SDL_Texture *tex;
    int nframes;
    int frame_px; /* source frame width == height */
} sdl_anim;
static sdl_anim *g_anim = NULL;        /* indexed by tile number */
static int g_anim_count = 0;
static boolean g_animate_wait = FALSE; /* keep rendering while idle */
#define SDL_ANIM_FRAME_MS 200

/* pixel geometry of the three stacked regions; computed once the real
   cell/tile sizes are known during sdl_init_nhwindows() */
static int g_map_top_px = 0;
static int g_status_top_px = 0;
static int g_window_w_px = 0, g_window_h_px = 0;
static int g_screen_cols = COLNO;  /* text columns that fit horizontally */
static int g_screen_rows = 0;      /* text rows that fit vertically */

/* leftmost map column currently visible; recomputed every frame in
   sdl_render_screen() from the player's position, clamped to keep the
   VIEW_COLS-wide window inside [0, COLNO) */
static int g_cam_x = 0;

/* winid of the single message/map/status window of each kind, once
   the core creates them; -1 until then */
static winid g_msg_win = WIN_ERR;
static winid g_map_win = WIN_ERR;
static winid g_status_win = WIN_ERR;

/* small glyph texture cache, keyed by ASCII char and NetHack color index */
static SDL_Texture *g_glyph_cache[128][CLR_MAX];

static const SDL_Color sdl_palette[CLR_MAX] = {
    { 0, 0, 0, 255 },       /* CLR_BLACK */
    { 197, 15, 31, 255 },   /* CLR_RED */
    { 19, 161, 14, 255 },   /* CLR_GREEN */
    { 193, 156, 0, 255 },   /* CLR_BROWN */
    { 0, 55, 218, 255 },    /* CLR_BLUE */
    { 136, 23, 152, 255 },  /* CLR_MAGENTA */
    { 58, 150, 221, 255 },  /* CLR_CYAN */
    { 204, 204, 204, 255 }, /* CLR_GRAY */
    { 118, 118, 118, 255 }, /* unused slot */
    { 249, 241, 165, 255 }, /* CLR_ORANGE (nb: reused slot, see color.h) */
    { 22, 198, 12, 255 },   /* CLR_BRIGHT_GREEN */
    { 249, 241, 165, 255 }, /* CLR_YELLOW */
    { 59, 120, 255, 255 },  /* CLR_BRIGHT_BLUE */
    { 180, 0, 158, 255 },   /* CLR_BRIGHT_MAGENTA */
    { 97, 214, 214, 255 },  /* CLR_BRIGHT_CYAN */
    { 242, 242, 242, 255 }, /* CLR_WHITE */
};

/* candidate monospace fonts to try, in order; macOS first since that's
   the primary dev target for this scaffold, then common Linux paths */
static const char *sdl_font_candidates[] = {
    "/System/Library/Fonts/Menlo.ttc",
    "/System/Library/Fonts/SFNSMono.ttf",
    "/System/Library/Fonts/Monaco.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    NULL
};

/* candidate tileset paths, tried relative to the current directory --
   which by the time init_nhwindows() runs is HACKDIR (see chdirx() in
   sys/unix/unixmain.c), hence the first entry; the rest are fallbacks
   for running straight out of an uninstalled checkout */
static const char *sdl_tile_candidates[] = {
    "unchozo32b.png",
    "tilesets/unchozo32b.png",
    "../tilesets/unchozo32b.png",
    "../../tilesets/unchozo32b.png",
    NULL
};

/* ------------------------------------------------------------------ */
/* small helpers                                                      */

static sdl_window *
sdl_win(winid wid)
{
    if (wid < 0 || wid >= SDL_MAXWIN || !sdl_windows[wid].inuse)
        panic("sdlwin: bad winid %d", wid);
    return &sdl_windows[wid];
}

static void
sdl_win_addline(sdl_window *w, const char *text)
{
    if (w->numlines >= w->linecap) {
        int newcap = w->linecap ? w->linecap * 2 : 32;
        if (newcap > SDL_MAXTEXTLINES)
            newcap = SDL_MAXTEXTLINES;
        w->lines = (char **) realloc(w->lines, newcap * sizeof(char *));
        w->linecap = newcap;
    }
    if (w->numlines >= SDL_MAXTEXTLINES) {
        /* drop oldest line to make room; message/scrollback windows
           are the only ones that can grow unbounded */
        free(w->lines[0]);
        memmove(&w->lines[0], &w->lines[1],
                (SDL_MAXTEXTLINES - 1) * sizeof(char *));
        w->numlines--;
    }
    w->lines[w->numlines++] = strdup(text ? text : "");
}

static void
sdl_win_setline(sdl_window *w, int row, const char *text)
{
    if (row < 0)
        return;
    while (w->numlines <= row)
        sdl_win_addline(w, "");
    free(w->lines[row]);
    w->lines[row] = strdup(text ? text : "");
}

/* ------------------------------------------------------------------ */
/* rendering                                                          */

static SDL_Texture *
sdl_glyph_texture(unsigned char ch, int coloridx)
{
    char s[2];
    SDL_Surface *surf;
    SDL_Texture *tex;

    if (ch < 32 || ch >= 127)
        ch = ' ';
    if (coloridx < 0 || coloridx >= CLR_MAX)
        coloridx = CLR_GRAY;
    if (g_glyph_cache[ch][coloridx])
        return g_glyph_cache[ch][coloridx];

    s[0] = (char) ch;
    s[1] = '\0';
    surf = TTF_RenderText_Blended(g_font, s, sdl_palette[coloridx]);
    if (!surf)
        return NULL;
    tex = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_FreeSurface(surf);
    g_glyph_cache[ch][coloridx] = tex;
    return tex;
}

/* raw pixel placement, both axes -- the primitive every other text
   helper below builds on */
static void
sdl_draw_glyph_px(int x_px, int y_px, unsigned char ch, int coloridx)
{
    SDL_Texture *tex = sdl_glyph_texture(ch, coloridx);
    SDL_Rect dst;
    if (!tex)
        return;
    dst.x = x_px;
    dst.y = y_px;
    dst.w = g_cell_w;
    dst.h = g_cell_h;
    SDL_RenderCopy(g_ren, tex, NULL, &dst);
}

/* col is in text-cell units; y_px is a raw pixel offset (not a row
   index) so message/status text and the tile-sized map can share the
   same drawing primitive despite having different row heights */
static void
sdl_draw_cell(int col, int y_px, unsigned char ch, int coloridx)
{
    sdl_draw_glyph_px(col * g_cell_w, y_px, ch, coloridx);
}

static void
sdl_draw_string(int col, int y_px, const char *s, int coloridx)
{
    int i;
    if (!s)
        return;
    for (i = 0; s[i] && col + i < g_screen_cols; i++)
        sdl_draw_cell(col + i, y_px, (unsigned char) s[i], coloridx);
}

/* draw up to `rows` lines of a scrolling text window, most recent last */
static void
sdl_draw_textwin(sdl_window *w, int top_px, int rows)
{
    int start = w->numlines - rows;
    int i, r;
    if (start < 0)
        start = 0;
    for (i = start, r = 0; i < w->numlines && r < rows; i++, r++)
        sdl_draw_string(0, top_px + r * g_cell_h, w->lines[i], CLR_WHITE);
}

/* blit the tile for `glyph` at pixel position (x_px, y_px); returns
   FALSE if no tile texture is loaded, so callers can fall back */
static boolean
sdl_draw_tile(int x_px, int y_px, int glyph)
{
    int tile;
    SDL_Rect src, dst;

    if (!g_tile_tex || glyph < 0 || glyph >= MAX_GLYPH)
        return FALSE;
    tile = glyph2tile[glyph];
    if (tile < 0 || tile >= total_tiles_used)
        return FALSE;

    if (g_anim && g_anim[tile].tex && g_anim[tile].nframes > 0) {
        int fpx = g_anim[tile].frame_px;
        int frame = (int) ((SDL_GetTicks() / SDL_ANIM_FRAME_MS)
                           % (Uint32) g_anim[tile].nframes);

        src.x = frame * fpx;
        src.y = 0;
        src.w = src.h = fpx;
        dst.x = x_px;
        dst.y = y_px;
        dst.w = g_tile_w;
        dst.h = g_tile_h;
        SDL_RenderCopy(g_ren, g_anim[tile].tex, &src, &dst);
        return TRUE;
    }

    src.x = (tile % SDL_TILES_PER_ROW) * g_tile_w;
    src.y = (tile / SDL_TILES_PER_ROW) * g_tile_h;
    src.w = g_tile_w;
    src.h = g_tile_h;
    dst.x = x_px;
    dst.y = y_px;
    dst.w = g_tile_w;
    dst.h = g_tile_h;
    SDL_RenderCopy(g_ren, g_tile_tex, &src, &dst);
    return TRUE;
}

static void
sdl_render_screen(void)
{
    int x, y;

    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);

    if (g_msg_win != WIN_ERR)
        sdl_draw_textwin(sdl_win(g_msg_win), 0, MSG_ROWS);

    if (g_map_win != WIN_ERR) {
        sdl_window *w = sdl_win(g_map_win);
        /* must match the cell size sdl_init_nhwindows() used to size
           the window: real tiles if loaded, otherwise plain text
           cells -- the two are not interchangeable dimensions */
        int cw = g_tile_tex ? g_tile_w : g_cell_w;
        int ch = g_tile_tex ? g_tile_h : g_cell_h;
        int max_cam = COLNO - VIEW_COLS;
        int xend;

        if (max_cam < 0)
            max_cam = 0;
        g_cam_x = u.ux - VIEW_COLS / 2;
        if (g_cam_x < 0)
            g_cam_x = 0;
        if (g_cam_x > max_cam)
            g_cam_x = max_cam;
        xend = g_cam_x + VIEW_COLS;
        if (xend > COLNO)
            xend = COLNO;

        for (y = 0; y < ROWNO; y++) {
            for (x = g_cam_x; x < xend; x++) {
                int glyph = w->glyphs ? w->glyphs[y * COLNO + x] : -1;
                int x_px, y_px;
                if (glyph < 0)
                    continue;
                x_px = (x - g_cam_x) * cw;
                y_px = g_map_top_px + y * ch;
                if (!sdl_draw_tile(x_px, y_px, glyph)) {
                    /* no tileset loaded: fall back to a colored
                       character, same lookup tty/curses use */
                    glyph_t gch;
                    int color;
                    unsigned special;
                    mapglyph(glyph, &gch, &color, &special, x, y, 0);
                    sdl_draw_glyph_px(x_px, y_px,
                                      (unsigned char) gch, color);
                }
            }
        }
    }

    if (g_status_win != WIN_ERR)
        sdl_draw_textwin(sdl_win(g_status_win), g_status_top_px, STATUS_ROWS);

    SDL_RenderPresent(g_ren);
}

/* ------------------------------------------------------------------ */
/* input                                                               */

/* Pump events until a key or (optionally) a map click arrives.
   Returns the translated character, or 0 if a mouse click was consumed
   instead (caller distinguishes via *got_click). */
static int
sdl_wait_input(coordxy *mx, coordxy *my, int *mod, boolean *got_click)
{
    SDL_Event ev;

    if (got_click)
        *got_click = FALSE;

    for (;;) {
        if (!SDL_WaitEventTimeout(&ev, 100)) {
            /* timeout, not an event: advance animations while idling
               for input -- but only at the map (menus and prompts
               paint their own screens that a re-render would clobber,
               so they leave g_animate_wait off) */
            if (g_animate_wait && g_anim_count > 0) {
                sdl_render_screen();
            }
            continue;
        }

        switch (ev.type) {
        case SDL_QUIT:
            /* Window closed: treat it like the terminal vanishing on
               a tty session -- hangup() saves any game in progress
               and cleans up the lock files, so closing the window is
               a safe way to quit (leaking locks eventually blocks new
               games with "There are too many hacks running now.") */
            hangup(1);
            return 0; /*NOTREACHED*/

        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_EXPOSED)
                sdl_render_screen();
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (mx && my) {
                int cw = g_tile_tex ? g_tile_w : g_cell_w;
                int ch = g_tile_tex ? g_tile_h : g_cell_h;
                float lx, ly;
                int col, row;
                /* event coordinates are in physical window pixels;
                   translate to the logical (pre-scaling) coordinate
                   space everything else in this file draws in */
                SDL_RenderWindowToLogical(g_ren, ev.button.x, ev.button.y,
                                           &lx, &ly);
                col = (int) lx / cw + g_cam_x;
                row = ((int) ly - g_map_top_px) / ch;
                if (row >= 0 && row < ROWNO && col >= 0 && col < COLNO) {
                    *mx = (coordxy) col;
                    *my = (coordxy) row;
                    if (mod)
                        *mod = (ev.button.button == SDL_BUTTON_RIGHT)
                                   ? CLICK_2 : CLICK_1;
                    if (got_click)
                        *got_click = TRUE;
                    return 0;
                }
            }
            break;

        case SDL_TEXTINPUT: {
            unsigned char c = (unsigned char) ev.text.text[0];
            if (c >= 32 && c < 127)
                return (int) c;
            break;
        }

        case SDL_KEYDOWN: {
            SDL_Keycode sym = ev.key.keysym.sym;
            Uint16 kmod = ev.key.keysym.mod;

            if (kmod & KMOD_CTRL) {
                if (sym >= SDLK_a && sym <= SDLK_z)
                    return (int) (sym - SDLK_a) + 1; /* ^A..^Z */
            }
            switch (sym) {
            case SDLK_ESCAPE:
                return 27;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                return 13;
            case SDLK_BACKSPACE:
                return 8;
            case SDLK_TAB:
                return 9;
            case SDLK_UP:
                return 'k';
            case SDLK_DOWN:
                return 'j';
            case SDLK_LEFT:
                return 'h';
            case SDLK_RIGHT:
                return 'l';
            default:
                break; /* printable keys arrive via SDL_TEXTINPUT instead */
            }
            break;
        }

        default:
            break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* lifecycle                                                          */

void
sdl_init_nhwindows(int *argc UNUSED, char **argv UNUSED)
{
    int i;
    const char *fontpath = NULL;
    int fh;
    int msg_h_px, map_h_px, status_h_px;
    SDL_Surface *tile_surf = NULL; /* held across window/renderer creation --
                                       SDL_CreateTextureFromSurface() needs
                                       a live renderer, which doesn't exist
                                       until after geometry is known */

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        panic("SDL_Init: %s", SDL_GetError());
    if (TTF_Init() != 0)
        panic("TTF_Init: %s", TTF_GetError());
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        raw_print("sdl port: SDL_image PNG support unavailable; "
                   "map will fall back to colored characters");

    for (i = 0; sdl_font_candidates[i]; i++) {
        g_font = TTF_OpenFont(sdl_font_candidates[i], 16);
        if (g_font) {
            fontpath = sdl_font_candidates[i];
            break;
        }
    }
    if (!g_font)
        panic("sdl port: no usable monospace font found; see "
              "win/sdl/Install.SDL2 for how to point this at one");

    TTF_SizeUTF8(g_font, "M", &g_cell_w, NULL);
    g_cell_w += CELL_PAD;
    fh = TTF_FontLineSkip(g_font);
    g_cell_h = fh > 0 ? fh : 16;

    /* try to find real tile art (surface only -- no renderer to turn
       it into a texture yet); sdl_draw_tile()/sdl_render_screen() fall
       back to colored characters per-glyph if this never succeeds, so
       a missing asset degrades gracefully rather than crashing */
    for (i = 0; sdl_tile_candidates[i]; i++) {
        SDL_Surface *surf = IMG_Load(sdl_tile_candidates[i]);
        if (!surf)
            continue;
        if (surf->w % SDL_TILES_PER_ROW != 0
            || (surf->h / (surf->w / SDL_TILES_PER_ROW))
                       * SDL_TILES_PER_ROW
                   < total_tiles_used) {
            /* wrong layout, or not enough tiles for this build's
               glyph set -- keep looking rather than draw garbage */
            SDL_FreeSurface(surf);
            continue;
        }
        tile_surf = surf;
        g_tile_w = g_tile_h = surf->w / SDL_TILES_PER_ROW;
        break;
    }

    msg_h_px = MSG_ROWS * g_cell_h;
    map_h_px = ROWNO * (tile_surf ? g_tile_h : g_cell_h);
    status_h_px = STATUS_ROWS * g_cell_h;
    g_map_top_px = msg_h_px;
    g_status_top_px = msg_h_px + map_h_px;
    g_window_w_px = VIEW_COLS * (tile_surf ? g_tile_w : g_cell_w);
    g_window_h_px = msg_h_px + map_h_px + status_h_px;

    /* g_window_w_px/h_px above is the *logical* game resolution (e.g.
       80 tile columns at 32px each = 2560px wide) -- often wider than
       an actual screen. Create the real OS window scaled down to fit
       the display, and let SDL_RenderSetLogicalSize() below transparently
       map every existing pixel-coordinate draw call onto it. */
    {
        SDL_Rect bounds;
        int win_w = g_window_w_px, win_h = g_window_h_px;

        if (SDL_GetDisplayUsableBounds(0, &bounds) == 0) {
            int max_w = (int) (bounds.w * 0.92);
            int max_h = (int) (bounds.h * 0.92);
            if (win_w > max_w || win_h > max_h) {
                double scale_w = (double) max_w / win_w;
                double scale_h = (double) max_h / win_h;
                double scale = (scale_w < scale_h) ? scale_w : scale_h;
                win_w = (int) (win_w * scale);
                win_h = (int) (win_h * scale);
            }
        }

        g_win = SDL_CreateWindow("UnNetHack",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  win_w, win_h,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    }
    if (!g_win)
        panic("SDL_CreateWindow: %s", SDL_GetError());

    g_ren = SDL_CreateRenderer(g_win, -1,
                                SDL_RENDERER_ACCELERATED
                                    | SDL_RENDERER_PRESENTVSYNC);
    if (!g_ren)
        g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    if (!g_ren)
        panic("SDL_CreateRenderer: %s", SDL_GetError());

    /* everything else in this file draws in g_window_w_px/h_px
       (logical) coordinates; this makes SDL scale that onto whatever
       actual window size was just created, letterboxing as needed */
    SDL_RenderSetLogicalSize(g_ren, g_window_w_px, g_window_h_px);

    if (tile_surf) {
        g_tile_tex = SDL_CreateTextureFromSurface(g_ren, tile_surf);
        SDL_FreeSurface(tile_surf);
    }

    /* animation overlays live in HACKDIR/anim (cwd is HACKDIR by now,
       same as the tileset search above); absence is normal */
    if (g_tile_tex) {
        DIR *dp = opendir("anim");

        if (dp) {
            struct dirent *de;

            g_anim = (sdl_anim *) calloc((size_t) total_tiles_used,
                                         sizeof (sdl_anim));
            /* smooth scaling for HD frames; static sheet tiles keep
               the default crisp nearest-neighbor look */
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
            while ((de = readdir(dp)) != 0) {
                int tidx;
                char apath[BUFSZ];
                SDL_Surface *as;

                if (sscanf(de->d_name, "%d.png", &tidx) != 1
                    || tidx < 0 || tidx >= total_tiles_used)
                    continue;
                Sprintf(apath, "anim/%s", de->d_name);
                as = IMG_Load(apath);
                if (!as)
                    continue;
                if (as->h > 0 && as->w >= as->h && (as->w % as->h) == 0
                    && !g_anim[tidx].tex) {
                    g_anim[tidx].tex =
                        SDL_CreateTextureFromSurface(g_ren, as);
                    if (g_anim[tidx].tex) {
                        g_anim[tidx].frame_px = as->h;
                        g_anim[tidx].nframes = as->w / as->h;
                        g_anim_count++;
                    }
                }
                SDL_FreeSurface(as);
            }
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            closedir(dp);
        }
    }

    g_screen_cols = g_window_w_px / g_cell_w;
    g_screen_rows = g_window_h_px / g_cell_h;

    SDL_StartTextInput();

    memset(sdl_windows, 0, sizeof sdl_windows);
    memset(g_glyph_cache, 0, sizeof g_glyph_cache);

    /* every windowport is responsible for setting this once its window
       is ready; pline() checks it and routes every single message to
       raw_print() (the terminal, not this window) until it's set --
       see src/pline.c around "if (!iflags.window_inited)" */
    iflags.window_inited = TRUE;

    nhUse(fontpath);
}

void
sdl_player_selection(void)
{
    /* Scaffold: leave role/race/gender/alignment however they were set
       on the command line, and let the core's normal "pick anything
       still unset at random" behavior fill in the rest. A real
       role/race/gender/alignment picker (see win/tty/wintty.c's
       tty_player_selection for the dialog this should eventually grow
       into) is deliberately out of scope for the initial scaffold. */
}

void
sdl_askname(void)
{
    sdl_getlin("Who are you?", plname);
    if (plname[0] == '\033')
        plname[0] = '\0';
}

void
sdl_get_nh_event(void)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT)
            hangup(1); /* save-and-exit, same as sdl_wait_input */
        if (ev.type == SDL_WINDOWEVENT
            && ev.window.event == SDL_WINDOWEVENT_EXPOSED)
            sdl_render_screen();
    }
}

void
sdl_exit_nhwindows(const char *str)
{
    int i;
    iflags.window_inited = FALSE;
    if (str)
        raw_print(str);
    for (i = 0; i < SDL_MAXWIN; i++)
        if (sdl_windows[i].inuse)
            sdl_destroy_nhwindow(i);
    for (i = 0; i < 128; i++) {
        int c;
        for (c = 0; c < CLR_MAX; c++)
            if (g_glyph_cache[i][c])
                SDL_DestroyTexture(g_glyph_cache[i][c]);
    }
    if (g_anim) {
        for (i = 0; i < total_tiles_used; i++)
            if (g_anim[i].tex)
                SDL_DestroyTexture(g_anim[i].tex);
        free(g_anim);
        g_anim = NULL;
        g_anim_count = 0;
    }
    if (g_tile_tex)
        SDL_DestroyTexture(g_tile_tex);
    if (g_font)
        TTF_CloseFont(g_font);
    if (g_ren)
        SDL_DestroyRenderer(g_ren);
    if (g_win)
        SDL_DestroyWindow(g_win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void
sdl_suspend_nhwindows(const char *str UNUSED)
{
    /* No real terminal to suspend to; nothing to do. */
}

void
sdl_resume_nhwindows(void)
{
    sdl_render_screen();
}

/* ------------------------------------------------------------------ */
/* windows                                                             */

winid
sdl_create_nhwindow(int type)
{
    int i;
    for (i = 0; i < SDL_MAXWIN; i++) {
        if (!sdl_windows[i].inuse) {
            sdl_window *w = &sdl_windows[i];
            memset(w, 0, sizeof *w);
            w->inuse = TRUE;
            w->type = type;
            if (type == NHW_MAP) {
                w->glyphs = (int *) malloc(ROWNO * COLNO * sizeof(int));
                w->bg_glyphs = (int *) malloc(ROWNO * COLNO * sizeof(int));
                sdl_clear_nhwindow(i);
                g_map_win = i;
            } else if (type == NHW_MESSAGE) {
                g_msg_win = i;
            } else if (type == NHW_STATUS) {
                g_status_win = i;
            }
            return i;
        }
    }
    panic("sdl port: out of window slots (SDL_MAXWIN=%d)", SDL_MAXWIN);
    return WIN_ERR;
}

void
sdl_clear_nhwindow(winid wid)
{
    sdl_window *w = sdl_win(wid);
    int i;

    if (w->type == NHW_MAP && w->glyphs) {
        for (i = 0; i < ROWNO * COLNO; i++) {
            w->glyphs[i] = -1;
            w->bg_glyphs[i] = -1;
        }
    }
    for (i = 0; i < w->numlines; i++)
        free(w->lines[i]);
    w->numlines = 0;
}

void
sdl_display_nhwindow(winid wid, boolean block)
{
    sdl_window *w = sdl_win(wid);

    sdl_render_screen();

    if (w->type == NHW_TEXT) {
        /* full-screen paged text (help, etc): show it and wait for any
           key before returning control to the map */
        boolean unused;
        sdl_draw_textwin(w, 0, g_screen_rows);
        SDL_RenderPresent(g_ren);
        (void) sdl_wait_input(NULL, NULL, NULL, &unused);
        sdl_render_screen();
    } else if (block && w->type == NHW_MESSAGE) {
        /* crude --More-- until this window gets its own dedicated
           pause tracking */
        boolean unused;
        sdl_draw_string(0, (MSG_ROWS - 1) * g_cell_h, "--More--", CLR_YELLOW);
        SDL_RenderPresent(g_ren);
        (void) sdl_wait_input(NULL, NULL, NULL, &unused);
    }
}

void
sdl_destroy_nhwindow(winid wid)
{
    sdl_window *w = sdl_win(wid);
    int i;

    for (i = 0; i < w->numlines; i++)
        free(w->lines[i]);
    free(w->lines);
    free(w->glyphs);
    free(w->bg_glyphs);
    for (i = 0; i < w->numitems; i++)
        free(w->items[i].str);
    free(w->items);
    free(w->menuprompt);

    if (wid == g_msg_win)
        g_msg_win = WIN_ERR;
    if (wid == g_map_win)
        g_map_win = WIN_ERR;
    if (wid == g_status_win)
        g_status_win = WIN_ERR;

    memset(w, 0, sizeof *w);
}

void
sdl_curs(winid wid, int x, int y)
{
    sdl_window *w = sdl_win(wid);
    w->cursx = (coordxy) x;
    w->cursy = (coordxy) y;
}

void
sdl_putstr(winid wid, int attr UNUSED, const char *text)
{
    sdl_window *w = sdl_win(wid);

    if (w->type == NHW_STATUS)
        sdl_win_setline(w, w->cursy, text);
    else
        sdl_win_addline(w, text);
}

void
#ifdef FILE_AREAS
sdl_display_file(const char *farea, const char *filename, boolean must_exist)
#else
sdl_display_file(const char *filename, boolean must_exist)
#endif
{
    dlb *f;
    winid wid;
    char line[BUFSZ];

#ifdef FILE_AREAS
    f = dlb_fopen_area(farea, filename, "r");
#else
    f = dlb_fopen(filename, "r");
#endif
    if (!f) {
        if (must_exist)
            pline("Cannot open \"%s\".", filename);
        return;
    }

    wid = sdl_create_nhwindow(NHW_TEXT);
    while (dlb_fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        sdl_putstr(wid, ATR_NONE, line);
    }
    (void) dlb_fclose(f);
    sdl_display_nhwindow(wid, TRUE);
    sdl_destroy_nhwindow(wid);
}

/* ------------------------------------------------------------------ */
/* menus                                                               */

void
sdl_start_menu(winid wid)
{
    sdl_window *w = sdl_win(wid);
    int i;
    for (i = 0; i < w->numitems; i++)
        free(w->items[i].str);
    w->numitems = 0;
    free(w->menuprompt);
    w->menuprompt = NULL;
    w->menu_in_progress = TRUE;
}

void
sdl_add_menu(winid wid, int glyph, int cnt UNUSED, const ANY_P *identifier,
             char accelerator, char group_accel UNUSED, int attr,
             const char *str, unsigned int presel)
{
    sdl_window *w = sdl_win(wid);
    sdl_menu_item *it;

    if (w->numitems >= w->itemcap) {
        int newcap = w->itemcap ? w->itemcap * 2 : 32;
        w->items = (sdl_menu_item *) realloc(w->items,
                                              newcap * sizeof(sdl_menu_item));
        w->itemcap = newcap;
    }
    it = &w->items[w->numitems++];
    memset(it, 0, sizeof *it);
    it->id = *identifier;
    it->count = -1;
    it->accelerator = accelerator;
    it->attr = attr;
    it->str = strdup(str ? str : "");
    it->glyph = glyph;
    it->presel = presel;
    it->selected = presel ? TRUE : FALSE;
}

void
sdl_end_menu(winid wid, const char *prompt)
{
    sdl_window *w = sdl_win(wid);
    char menu_ch = 'a';
    int i;

    /* add_menu()'s caller is allowed to pass accelerator == 0 to mean
       "window port picks one" -- every other port assigns a-z then
       A-Z to selectable items at menu-finalize time (see tty_end_menu
       in win/tty/wintty.c for the reference sequence); without this,
       items rendered with no visible/pressable letter at all */
    for (i = 0; i < w->numitems; i++) {
        if (w->items[i].id.a_void != NULL && !w->items[i].accelerator) {
            w->items[i].accelerator = menu_ch;
            if (menu_ch++ == 'z')
                menu_ch = 'A';
        }
    }

    w->menuprompt = strdup(prompt ? prompt : "");
    w->menu_in_progress = FALSE;
}

int
sdl_select_menu(winid wid, int how, MENU_ITEM_P **selected)
{
    sdl_window *w = sdl_win(wid);
    int cursor = 0, i, n;
    boolean done = FALSE, cancelled = FALSE;

    *selected = NULL;
    if (w->numitems == 0)
        return 0;
    /* start the cursor on the first selectable item */
    while (cursor < w->numitems && w->items[cursor].id.a_void == NULL)
        cursor++;
    if (cursor >= w->numitems)
        cursor = 0;

    while (!done) {
        int row = 0;
        SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
        SDL_RenderClear(g_ren);
        if (w->menuprompt && w->menuprompt[0])
            sdl_draw_string(0, row++ * g_cell_h, w->menuprompt, CLR_WHITE);
        for (i = 0; i < w->numitems && row < g_screen_rows; i++, row++) {
            char line[BUFSZ];
            boolean selectable = (w->items[i].id.a_void != NULL);
            const char *mark = selectable
                                    ? (w->items[i].selected ? "+" : "-")
                                    : " ";
            Sprintf(line, "%c %s %s",
                    w->items[i].accelerator ? w->items[i].accelerator : ' ',
                    selectable ? mark : " ", w->items[i].str);
            sdl_draw_string(1, row * g_cell_h,
                             (i == cursor && selectable) ? "->" : "",
                             CLR_WHITE);
            sdl_draw_string(4, row * g_cell_h, line,
                             (i == cursor) ? CLR_YELLOW : CLR_WHITE);
        }
        SDL_RenderPresent(g_ren);

        {
            boolean click;
            int ch = sdl_wait_input(NULL, NULL, NULL, &click);
            if (click)
                continue;

            if (ch == 27) {
                cancelled = TRUE;
                done = TRUE;
            } else if (ch == 13 || ch == ' ') {
                if (how == PICK_ONE) {
                    if (w->items[cursor].id.a_void != NULL) {
                        w->items[cursor].selected = TRUE;
                        done = TRUE;
                    }
                } else if (how == PICK_ANY) {
                    if (ch == ' ' && w->items[cursor].id.a_void != NULL)
                        w->items[cursor].selected = !w->items[cursor].selected;
                    else if (ch == 13)
                        done = TRUE;
                } else {
                    done = TRUE;
                }
            } else if (ch == 'j' || ch == 14 /* ^N */) {
                /* bounded scan, not do/while: a display-only menu can
                   have zero selectable items, which would otherwise
                   spin forever looking for one */
                int next = cursor, tries;
                for (tries = 0; tries < w->numitems; tries++) {
                    next = (next + 1) % w->numitems;
                    if (w->items[next].id.a_void != NULL) {
                        cursor = next;
                        break;
                    }
                }
            } else if (ch == 'k' || ch == 16 /* ^P */) {
                int next = cursor, tries;
                for (tries = 0; tries < w->numitems; tries++) {
                    next = (next - 1 + w->numitems) % w->numitems;
                    if (w->items[next].id.a_void != NULL) {
                        cursor = next;
                        break;
                    }
                }
            } else {
                for (i = 0; i < w->numitems; i++) {
                    if (w->items[i].accelerator == (char) ch
                        && w->items[i].id.a_void != NULL) {
                        if (how == PICK_ONE) {
                            w->items[i].selected = TRUE;
                            done = TRUE;
                        } else if (how == PICK_ANY) {
                            w->items[i].selected = !w->items[i].selected;
                        }
                        break;
                    }
                }
            }
        }
    }

    if (cancelled) {
        sdl_render_screen();
        return -1;
    }

    for (i = 0, n = 0; i < w->numitems; i++)
        if (w->items[i].selected)
            n++;

    if (n > 0) {
        *selected = (MENU_ITEM_P *) malloc(n * sizeof(MENU_ITEM_P));
        for (i = 0, n = 0; i < w->numitems; i++) {
            if (w->items[i].selected) {
                (*selected)[n].item = w->items[i].id;
                (*selected)[n].count = -1;
                n++;
            }
        }
    }

    sdl_render_screen();
    return n;
}

void
sdl_update_inventory(void)
{
    /* Only meaningful for ports that keep a persistent inventory
       window on screen (WC_PERM_INVENT); this scaffold doesn't
       advertise that capability. */
}

/* ------------------------------------------------------------------ */
/* misc required entry points                                         */

void
sdl_mark_synch(void)
{
}

void
sdl_wait_synch(void)
{
    sdl_render_screen();
}

#ifdef CLIPPING
void
sdl_cliparound(int x UNUSED, int y UNUSED)
{
    /* No-op: sdl_render_screen() recomputes g_cam_x from u.ux every
       frame instead of relying on this hint, which keeps the viewport
       correct even when core moves the player without calling
       cliparound (e.g. teleport) or calls it before glyphs update. */
}
#endif

void
sdl_print_glyph(winid wid, coordxy x, coordxy y, int glyph, int bg_glyph)
{
    sdl_window *w = sdl_win(wid);
    if (!w->glyphs || x < 0 || x >= COLNO || y < 0 || y >= ROWNO)
        return;
    w->glyphs[y * COLNO + x] = glyph;
    w->bg_glyphs[y * COLNO + x] = bg_glyph;
}

void
sdl_raw_print(const char *str)
{
    fprintf(stderr, "%s\n", str ? str : "");
}

void
sdl_raw_print_bold(const char *str)
{
    sdl_raw_print(str);
}

int
sdl_nhgetch(void)
{
    boolean click;
    int ch;
    sdl_render_screen();
    g_animate_wait = TRUE;
    do {
        ch = sdl_wait_input(NULL, NULL, NULL, &click);
    } while (click); /* nhgetch() callers don't want mouse events */
    g_animate_wait = FALSE;
    return ch;
}

int
sdl_nh_poskey(coordxy *x, coordxy *y, int *mod)
{
    boolean click;
    int ch;
    sdl_render_screen();
    g_animate_wait = TRUE;
    ch = sdl_wait_input(x, y, mod, &click);
    g_animate_wait = FALSE;
    return click ? 0 : ch;
}

void
sdl_nhbell(void)
{
    SDL_SetRenderDrawColor(g_ren, 255, 255, 255, 255);
    SDL_RenderClear(g_ren);
    SDL_RenderPresent(g_ren);
    SDL_Delay(40);
    sdl_render_screen();
}

int
sdl_doprev_message(void)
{
    /* No dedicated message-history scrollback yet; the message window
       already keeps its full scrollback in sdl_windows[], a viewer
       just isn't wired to a key here yet. */
    return 0;
}

char
sdl_yn_function(const char *question, const char *choices, char def)
{
    char qbuf[BUFSZ];
    int ch;
    boolean click;

    Sprintf(qbuf, "%s ", question ? question : "");
    sdl_draw_string(0, (MSG_ROWS - 1) * g_cell_h, qbuf, CLR_WHITE);
    SDL_RenderPresent(g_ren);

    for (;;) {
        ch = sdl_wait_input(NULL, NULL, NULL, &click);
        if (click)
            continue;
        if (ch == 13 && def)
            return def;
        if (ch == 27)
            return (choices && strchr(choices, 'q')) ? 'q' : def;
        if (choices && strchr(choices, ch))
            return (char) ch;
        if (!choices)
            return (char) ch;
    }
}

void
sdl_getlin(const char *question, char *input)
{
    char buf[BUFSZ];
    int len = 0;
    boolean click;

    buf[0] = '\0';
    for (;;) {
        char line[BUFSZ];
        int ch;

        Sprintf(line, "%s %s", question ? question : "", buf);
        SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
        SDL_RenderClear(g_ren);
        sdl_draw_string(0, (MSG_ROWS - 1) * g_cell_h, line, CLR_WHITE);
        SDL_RenderPresent(g_ren);

        ch = sdl_wait_input(NULL, NULL, NULL, &click);
        if (click)
            continue;
        if (ch == 13) {
            Strcpy(input, buf);
            return;
        }
        if (ch == 27) {
            input[0] = '\033';
            input[1] = '\0';
            return;
        }
        if (ch == 8 || ch == 127) {
            if (len > 0)
                buf[--len] = '\0';
            continue;
        }
        if (ch >= 32 && ch < 127 && len < BUFSZ - 1) {
            buf[len++] = (char) ch;
            buf[len] = '\0';
        }
    }
}

int
sdl_get_ext_cmd(void)
{
    char buf[BUFSZ];
    int i, match = -1, nmatches = 0;
    size_t len;

    sdl_getlin("# extended command:", buf);
    if (buf[0] == '\033' || !buf[0])
        return -1;
    len = strlen(buf);
    for (i = 0; extcmdlist[i].ef_txt; i++) {
        if ((extcmdlist[i].flags & WIZMODECMD) && !wizard)
            continue;
        if (!strcmpi(buf, extcmdlist[i].ef_txt))
            return i; /* exact match wins outright */
        if (!strncmpi(buf, extcmdlist[i].ef_txt, len)) {
            match = i;
            nmatches++;
        }
    }
    if (nmatches == 1)
        return match;
    pline(nmatches ? "Ambiguous extended command \"%s\"."
                   : "Unknown extended command \"%s\".", buf);
    return -1;
}

void
sdl_number_pad(int state UNUSED)
{
}

void
sdl_delay_output(void)
{
    sdl_render_screen();
    SDL_Delay(50);
}

void
sdl_start_screen(void)
{
}

void
sdl_end_screen(void)
{
}

void
sdl_outrip(winid wid, int how UNUSED)
{
    sdl_putstr(wid, ATR_NONE, "");
    sdl_putstr(wid, ATR_NONE, "  R.I.P.");
    sdl_putstr(wid, ATR_NONE, "");
    sdl_display_nhwindow(wid, TRUE);
}

/* Stubs for capability slots that only exist in the window_procs struct
   when POSITIONBAR / CHANGE_COLOR are defined; neither is enabled by
   default in this tree. Kept as real typed no-ops (rather than
   reusing do.c's donull(), which has an unrelated signature) so this
   file stays correct if either capability is ever turned on. */
#ifdef POSITIONBAR
void
sdl_update_positionbar(char *bar UNUSED)
{
}
#endif

#ifdef CHANGE_COLOR
void
sdl_change_color(int a UNUSED, long b UNUSED, int c UNUSED)
{
}

char *
sdl_get_color_string(void)
{
    return NULL;
}
#endif

/* ------------------------------------------------------------------ */

struct window_procs sdl_procs = {
    "sdl",
    WC_COLOR | WC_MOUSE_SUPPORT,
    0L,
    sdl_init_nhwindows,
    sdl_player_selection,
    sdl_askname,
    sdl_get_nh_event,
    sdl_exit_nhwindows,
    sdl_suspend_nhwindows,
    sdl_resume_nhwindows,
    sdl_create_nhwindow,
    sdl_clear_nhwindow,
    sdl_display_nhwindow,
    sdl_destroy_nhwindow,
    sdl_curs,
    sdl_putstr,
    sdl_display_file,
    sdl_start_menu,
    sdl_add_menu,
    sdl_end_menu,
    sdl_select_menu,
    genl_message_menu,
    sdl_update_inventory,
    sdl_mark_synch,
    sdl_wait_synch,
#ifdef CLIPPING
    sdl_cliparound,
#endif
#ifdef POSITIONBAR
    sdl_update_positionbar,
#endif
    sdl_print_glyph,
    sdl_raw_print,
    sdl_raw_print_bold,
    sdl_nhgetch,
    sdl_nh_poskey,
    sdl_nhbell,
    sdl_doprev_message,
    sdl_yn_function,
    sdl_getlin,
    sdl_get_ext_cmd,
    sdl_number_pad,
    sdl_delay_output,
#ifdef CHANGE_COLOR
    sdl_change_color,
#ifdef MAC
    donull,
    donull,
#endif
    sdl_get_color_string,
#endif
    sdl_start_screen,
    sdl_end_screen,
    sdl_outrip,
    genl_preference_update,
};

/*sdlmain.c*/
