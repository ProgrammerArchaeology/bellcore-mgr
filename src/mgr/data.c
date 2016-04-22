/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* static data items for window manager */

/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "defs.h"

#include "cut.h"
#include "destroy.h"
#include "do_button.h"
#include "font_subs.h"
#include "move.h"
#include "shape.h"
/*}}}  */

/* aarg! */

static void nothing(void) {}

/* menus */

const char *active_menu[] = { /* active-window menu */
#ifdef STRETCH                /* first menu item: something you can safely click by accident */
  "move",
  "stretch",
#else
  "reshape",
  "move",
#endif
  "cut",
  "paste",
  "bury",
  "- - - -",
  "destroy",
  NULL
};

const char *main_menu[] = { /* primary menu */
  "new window",
  "redraw",
  "quit",
  NULL
};

const char *full_menu[] = { /* primary menu  - no more windows allowed */
  "redraw",
  "quit",
  NULL
};

const char *quit_menu[] = { /* to verify quit */
  "cancel",
  "lock screen",
  "suspend",
  "- - - -",
  "really quit",
  NULL
};

/* menu functions - these have a 1-1 correspondence with the menu items */

function main_functions[] = {
  new_window,
  redraw,
  quit,
  (function)0
};

function full_functions[] = {
  redraw,
  quit,
  (function)0
};

function active_functions[] = {
#ifdef STRETCH
  move_window,
  stretch_window,
#else
  shape_window,
  move_window,
#endif
  rubber_band_cut,
  paste,
  hide_win,
  nothing,
  destroy_window,
  (function)0
};

/* default font info */

const char *font_dir = FONTDIR;
char *fontlist[MAXFONT];

/* default icon info */
const char *icon_dir = ICONDIR;

/* color index map for fixed colors */

unsigned char color_map[COLORMAP_SIZE] = {
  0,   /* logo fg */
  255, /* logo bg */
  0,   /* cr   fg */
  255, /* cr   bg (unused) */
  255, /* menu fg */
  0,   /* menu bg */
  255, /* root pattern fg */
  0    /* root pattern bg */
};

BITMAP *m_rop;                        /* current mouse bit map */
BITMAP *mouse_save;                   /* where to keep what cursor's on */
int next_window = 0;                  /* next available window count */
struct font *font;                    /* default font */
BITMAP *screen, *prime;               /* default screen */
WINDOW *active = NULL;                /* window connected to keyboard */
WINDOW *last_active = NULL;           /* previous window connected to keyboard */
int button_state = 0;                 /* state of the mouse buttons */
int mouse, mousex, mousey;            /* mouse fd, x-coord, y-coord */
int debug = 0;                        /* ==1 for debug prints */
int mouse_on = 0;                     /* 1 iff mouse track is on */
char *snarf = NULL;                   /* place to keep snarfed text */
char *message = NULL;                 /* place to keep message */
int id_message = 0;                   /* id of message sender */
unsigned int init_flags = INIT_FLAGS; /* initial flags for new windows */
fd_set mask;                          /* process mask for select */
fd_set to_poll;                       /* processes with non-processed but
							* already read data */
#ifdef DEBUG
char debug_level[] = "                                "; /* debug flags */
#endif
