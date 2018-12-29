/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* Terminal emulator */
/*}}}  */
/*{{{  #includes*/
#include <sys/time.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "clip.h"
#include "defs.h"
#include "event.h"
#include "menu.h"

#include "proto.h"
#include "Write.h"
#include "border.h"
#include "colormap.h"
#include "do_event.h"
#include "do_menu.h"
#include "down_load.h"
#include "font_subs.h"
#include "get_font.h"
#include "get_info.h"
#include "get_menus.h"
#include "icon_server.h"
#include "intersect.h"
#include "kbd.h"
#include "new_window.h"
#include "scroll.h"
#include "shape.h"
#include "subs.h"
#include "update.h"
#include "win_make.h"
#include "win_stack.h"
#include "win_subs.h"
/*}}}  */
/*{{{  #defines*/
/* macros for putting a character on a window */

#define DO_CHAR(font, c) \
  ((font)->glyph[c])

#define PUT_CHAR(dest, x, y, font, op, c)                \
  bit_blit(dest, x, y - fsizehigh, fsizewide, fsizehigh, \
      op, DO_CHAR(font, c), 0, 0)

/* fix the border color */

#define BORDER(win) \
  ((win == active) ? border(win, BORDER_FAT) : border(win, BORDER_THIN))

#define BG_OP PUTOP(BIT_CLR, win->style)

/* fsleep is experimental */

#define fsleep()               \
  {                            \
    struct timeval time;       \
    time.tv_sec = 0;           \
    time.tv_usec = 330000;     \
    select(0, 0, 0, 0, &time); \
  }

#define B_SIZE8(w, h, d) ((h) * ((((w * d) + 7L) & ~7L) >> 3))
/*}}}  */

/*{{{  variables*/
rect clip; /* clipping rectangle */
/*}}}  */

/*{{{  set_winsize*/
static void
set_winsize(int fd, int rows, int cols, int ypixel, int xpixel)
{
  struct winsize size;

  size.ws_row = rows;
  size.ws_col = cols;
  size.ws_xpixel = xpixel;
  size.ws_ypixel = ypixel;
  ioctl(fd, TIOCSWINSZ, &size);
  dbgprintf('t', (stderr, "SWINSZ ioctl %dx%d\n", rows, cols));
}
/*}}}  */
/*{{{  standout*/
static void standout(WINDOW *win)
{
  if (win->flags & W_STANDOUT)
    return;

#ifdef COLORSTANDOUT
  if (BIT_DEPTH(win->window) > 1 && GETFCOLOR(~win->style) != GETBCOLOR(win->style))
    win->style = PUTFCOLOR(win->style, GETFCOLOR(~win->style));
  else
#endif /* COLORSTANDOUT */
    win->style = PUTOP(BIT_NOT(win->style), win->style);
  win->flags |= W_STANDOUT;
}
/*}}}  */
/*{{{  standend*/
static void standend(WINDOW *win)
{
  if (!(win->flags & W_STANDOUT))
    return;

#ifdef COLORSTANDOUT
  if (BIT_DEPTH(win->window) > 1 && GETFCOLOR(~win->style) != GETBCOLOR(win->style))
    win->style = PUTFCOLOR(win->style, GETFCOLOR(~win->style));
  else
#endif /* COLORSTANDOUT */
    win->style = PUTOP(BIT_NOT(win->style), win->style);
  win->flags &= ~W_STANDOUT;
}
/*}}}  */

/*{{{  set_size -- set the kernel's idea of the screen size*/
void set_size(WINDOW *win)
{
  if (win == NULL)
    return;

  if (win->flags & W_NOREPORT)
    return; /* just return if user requested */
  if (win->text.wide > 0) {
    set_winsize(active->to_fd, win->text.high / FSIZE(high), win->text.wide / FSIZE(wide), 0, 0);
  } else {
    set_winsize(win->to_fd, BIT_HIGH(win->window) / FSIZE(high), BIT_WIDE(win->window) / FSIZE(wide), 0, 0);
  }
}
/*}}}  */
/*{{{  put_window -- send a string to a window, interpret ESCs, return # of processed character*/
int put_window(WINDOW *win, const char *buff, int buff_count)
{
  /*{{{  variables*/
  BITMAP *window;           /* bitmap to update */
  BITMAP *text = NULL;      /* current text region */
  int indx;                 /* index into buff */
  int cnt;                  /* # of esc. numbers */
  char c;                   /* current char */
  int done = 0;             /* set to 1 to exit */
  int bell = 0;             /* 1 if screen flashed once */
  int sub_window = 0;       /* sub window created */
  int fsizehigh, fsizewide; /* variables to save deref. */
  int offset = 0;           /* font glyph offset */
  char tbuff[40];           /* tmp space for replies */
  /*}}}  */

  /*{{{  set up environment*/
  if (win->flags & W_ACTIVE)
    window = win->window;
  else {
    window = bit_create(win->save, win->borderwid, win->borderwid, BIT_WIDE(win->window), BIT_HIGH(win->window));
    sub_window++;
  }

  if (window == NULL) {
    perror("Bit_create failed for window");
    return (0);
  }
  /*}}}  */
  /*{{{  avoid repeated dereferencing of pointers*/

  fsizehigh = FSIZE(high);
  fsizewide = FSIZE(wide);

  if (win->flags & W_SPECIAL) {
    if (win->flags & W_UNDER)
      offset = MAXGLYPHS;
    if (win->flags & W_BOLD)
      offset += 2 * MAXGLYPHS;
  }

  if (Do_clip()) {
    Set_clipall();
    Set_cliplow(win->x + win->text.x, win->y + win->text.y - fsizehigh);
  }

  if (win->text.wide)
    text = bit_create(window, win->text.x, win->text.y, win->text.wide, win->text.high);
  if (text == NULL)
    text = window;

  if (win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)) {
    MOUSE_OFF(screen, mousex, mousey);
  }

  if (win == active)
    cursor_off();

  /*}}}  */
  /*{{{  do each character*/
  for (indx = 0; c = *buff++, indx < buff_count && !done; indx++)
    switch (win->flags & W_STATE) {
    /*{{{  W_TEXT -- down load a text string*/
    case W_TEXT:
      cnt = win->esc_cnt;
      win->snarf[win->esc[TEXT_COUNT]++] = c;
      if (win->esc[TEXT_COUNT] >= win->esc[cnt]) {
        win->flags &= ~W_TEXT;
        if (win->snarf && win->code != T_BITMAP && win->code != T_GRUNCH) {
          win->snarf[win->esc[TEXT_COUNT]] = '\0';
          trans(win->snarf);
        }
        down_load(win, window, text);
        done++;
      }
      break;
    /*}}}  */
    /*{{{  W_ESCAPE -- process an escape code*/
    case W_ESCAPE:
      win->flags &= ~(W_ESCAPE);
      cnt = win->esc_cnt;
      switch (c) {
      /*{{{  ESC          -- turn on escape mode*/
      case ESC:
        win->flags |= W_ESCAPE;
        win->esc_cnt = 0;
        win->esc[0] = 0;
        break;
      /*}}}  */
      /*{{{  0, 1, 2, 3, 4, 5, 6, 7, 8, 9*/
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        int n = win->esc[win->esc_cnt];

        if (n >= 0)
          n = n * 10 + (c - '0');
        else
          n = n * 10 - (c - '0');
        win->flags |= W_ESCAPE;
        if (win->flags & W_MINUS && n != 0) {
          n = -n;
          win->flags &= ~(W_MINUS);
        }
        win->esc[win->esc_cnt] = n;
      } break;
      /*}}}  */
      /*{{{  E_SEP1, E_SEP2 field separators*/
      case E_SEP1:
      case E_SEP2:
        if (win->esc_cnt + 1 < MAXESC)
          win->esc_cnt++;
        win->esc[win->esc_cnt] = 0;
        win->flags &= ~(W_MINUS);
        win->flags |= W_ESCAPE;
        break;
      /*}}}  */
      /*{{{  E_MINUS      -- set the MINUS flag*/
      case E_MINUS:
        win->flags |= (W_ESCAPE | W_MINUS);
        break;
      /*}}}  */
      /*{{{  E_NULL       -- do nothing*/
      case E_NULL:
        done++;
        break;
      /*}}}  */
      /*{{{  E_ADDLINE    -- add a new line*/
      case E_ADDLINE:
        if (*win->esc) {
          int count = *win->esc;
          scroll(win, text, win->y - fsizehigh, T_HIGH, -count * (fsizehigh), BG_OP);
        } else {
          scroll(win, text, win->y - fsizehigh, T_HIGH, -(fsizehigh), BG_OP);
        }
        done++;
        break;
      /*}}}  */
      /*{{{  E_ADDCHAR    -- insert a character*/
      case E_ADDCHAR: {
        int wide = (fsizewide) * (*win->esc ? *win->esc : 1);
        if (wide + win->x > T_WIDE)
          wide = T_WIDE - win->x;
        bit_blit(text, win->x + wide, win->y - fsizehigh,
            T_WIDE - win->x - wide, fsizehigh, BIT_SRC,
            text, win->x, win->y - fsizehigh);
        bit_blit(text, win->x, win->y - fsizehigh, wide,
            fsizehigh, BG_OP, 0, 0, 0);
      } break;
      /*}}}  */
      /*{{{  E_DELETELINE -- delete a line*/
      case E_DELETELINE: /* delete a line */
        if (*win->esc) {
          int count = *win->esc;
          scroll(win, text, win->y - fsizehigh, T_HIGH, count * fsizehigh,
              BG_OP);
        } else {
          scroll(win, text, win->y - fsizehigh, T_HIGH, fsizehigh,
              BG_OP);
        }
        done++;
        break;
      /*}}}  */
      /*{{{  E_DELETECHAR -- delete a character*/
      case E_DELETECHAR: {
        int wide = (fsizewide) * (*win->esc ? *win->esc : 1);
        if (wide + win->x > T_WIDE)
          wide = T_WIDE - win->x;
        bit_blit(text, win->x, win->y - fsizehigh,
            T_WIDE - win->x - wide, fsizehigh, BIT_SRC,
            text, win->x + wide, win->y - fsizehigh);
        bit_blit(text, T_WIDE - wide, win->y - fsizehigh, wide,
            fsizehigh, BG_OP, 0, 0, 0);
      } break;
      /*}}}  */
      /*{{{  E_UPLINE     -- up 1 line*/
      case E_UPLINE: /* up 1 line */
#ifdef FRACCHAR
        if (cnt > 0) { /* move up fractions of a character line */
          int div = win->esc[1] == 0 ? 1 : win->esc[1];
          int n = win->esc[0] * fsizehigh / div;
          if (win->y > n) {
            win->y -= n;
            if (Do_clip())
              Set_cliplow(10000, win->y + win->text.y - fsizehigh);
          }
          break;
        }
#endif
        if (win->y > fsizehigh)
          win->y -= fsizehigh;
        if (Do_clip())
          Set_cliplow(10000, win->y + win->text.y - fsizehigh);
        break;
      /*}}}  */
      /*{{{  E_RIGHT      -- right 1 line*/
      case E_RIGHT:
#ifdef FRACCHAR
        if (cnt > 0) { /* move right/left a fraction of a character */
          int div = win->esc[1] == 0 ? 1 : win->esc[1];
          int n = win->esc[0] * fsizewide / div;
          win->x += n;
          if (win->x < 0)
            win->x = 0;
          break;
        }
#endif
        win->x += fsizewide;
        break;
      /*}}}  */
      /*{{{  E_DOWN       -- down 1 line*/
      case E_DOWN: /* down 1 line */
#ifdef FRACCHAR
        if (cnt > 0) { /* move down a fraction of a character */
          int div = win->esc[1] == 0 ? 1 : win->esc[1];
          int n = win->esc[0] * fsizehigh / div;

          if (win->y + n > T_HIGH) {
            scroll(win, text, 0, T_HIGH, n, BG_OP);
            done++;
          } else {
            win->y += n;
            if (Do_clip())
              Set_cliphigh(0, win->y + win->text.y);
          }
          break;
        }
#endif
        if (win->y + fsizehigh > T_HIGH) {
          scroll(win, text, 0, T_HIGH, fsizehigh, BG_OP);
          done++;
        } else {
          win->y += fsizehigh;
          if (Do_clip())
            Set_cliphigh(0, win->y + win->text.y);
        }
        break;
      /*}}}  */
      /*{{{  E_FCOLOR     -- set foreground color*/
      case E_FCOLOR:
        if (win->flags & W_STANDOUT) {
          standend(win);
          win->style = win->flags & W_REVERSE ? PUTBCOLOR(win->style, *win->esc) : PUTFCOLOR(win->style, *win->esc);
          standout(win);
        } else
          win->style = win->flags & W_REVERSE ? PUTBCOLOR(win->style, *win->esc) : PUTFCOLOR(win->style, *win->esc);
        BORDER(win);
        break;
      /*}}}  */
      /*{{{  E_BCOLOR     -- set background color*/
      case E_BCOLOR:
        win->style = win->flags & W_REVERSE ? PUTFCOLOR(win->style, *win->esc) : PUTBCOLOR(win->style, *win->esc);
        BORDER(win);
        break;
      /*}}}  */
      /*{{{  E_STANDOUT   -- inverse video (characters)*/
      case E_STANDOUT:
        standout(win);
        break;
      /*}}}  */
      /*{{{  E_STANDEND   -- normal video (characters)*/
      /* standend also sets character attributes */
      case E_STANDEND: {
        int mode = *win->esc;

        if (mode) {
          enhance_font(win->font);
          done++;
        }
        offset = 0;
        if (mode & 1) { /* standout */
          standout(win);
          offset = 1;
        } else {
          standend(win);
        }
        if (mode & 2) { /* bold */
          win->flags |= W_BOLD;
          offset |= 2;
        } else {
          win->flags &= ~W_BOLD;
        }
        if (mode & 4) { /* underline */
          win->flags |= W_UNDER;
          offset |= 4;
        } else {
          win->flags &= ~W_UNDER;
        }
        offset *= MAXGLYPHS;

        break;
      }
      /*}}}  */
      /*{{{  E_CLEAREOL   -- clear to end of line*/
      case E_CLEAREOL: /* clear to end of line */
        bit_blit(text, win->x, win->y - fsizehigh, T_WIDE - win->x, fsizehigh, BG_OP, 0, 0, 0);
        if (Do_clip())
          Set_cliphigh(BIT_WIDE(win->window), 0);
        break;
      /*}}}  */
      /*{{{  E_CLEAREOS   -- clear to end of window*/
      case E_CLEAREOS: /* clear to end of window */
        bit_blit(text, win->x, win->y - fsizehigh, T_WIDE - win->x, fsizehigh, BG_OP, 0, 0, 0);
        bit_blit(text, 0, win->y, T_WIDE, T_HIGH - win->y, BG_OP, 0, 0, 0);
        if (Do_clip())
          Set_cliphigh(BIT_WIDE(win->window), BIT_HIGH(window));
        break;
      /*}}}  */
      /*{{{  E_SETCURSOR  -- set the character cursor*/
      case E_SETCURSOR:
        win->curs_type = *win->esc;
        break;
      /*}}}  */
      /*{{{  E_BLEEP      -- highlight a section of the screen*/
      case E_BLEEP:
        if (cnt > 2) {
          int *p = win->esc;
          if (p[0] < 0 || p[1] < 0)
            break;
          p[2] = BETWEEN(1, p[2], BIT_WIDE(screen) - 1);
          p[3] = BETWEEN(1, p[3], BIT_WIDE(screen) - 1);
          bit_blit(screen, p[0], p[1], p[2], p[3], BIT_NOT(BIT_DST), 0, 0, 0);
          fsleep();
          bit_blit(screen, p[0], p[1], p[2], p[3], BIT_NOT(BIT_DST), 0, 0, 0);
          done++;
        }
        break;
      /*}}}  */
      /*{{{  E_FONT       -- pick a new font*/
      case E_FONT:
        win->flags &= ~W_SNARFABLE;
        win->flags &= ~W_SPECIAL; /* reset bold or underline */
        offset = 0;
        if (cnt > 0) {
          win->esc[TEXT_COUNT] = 0;
          if (win->esc[cnt] > 0 && (win->snarf = malloc(win->esc[cnt] + 1)) != 0)
            win->flags |= W_TEXT;
          win->code = T_FONT;
          break;
        }

        {
          int font_count = win->esc[cnt];
          int baseline = FSIZE(baseline);

          win->font = Get_font(font_count);
          fsizehigh = FSIZE(high);
          fsizewide = FSIZE(wide);
          win->y += FSIZE(baseline) - baseline;
          if (win->y < fsizehigh) {
            scroll(win, text, win->y - fsizehigh, T_HIGH, win->y - fsizehigh, BG_OP);
            win->y = fsizehigh;
            done++;
          }
        }
        set_size(win);
        break;
      /*}}}  */
      /*{{{  E_MOUSE      -- move the mouse or change cursor shape */
      case E_MOUSE:
        if (cnt == 0 || (cnt == 1 && win == active)) {
          int mouse_was_on = mouse_on;
          MOUSE_OFF(screen, mousex, mousey);

          if (cnt == 0) {
            /* change mouse cursor shape */
            int bn = win->esc[0];

            bit_destroy(win->cursor);
            if (bn > 0 && bn <= MAXBITMAPS
                && cursor_ok(win->bitmaps[bn - 1])
                && (win->cursor = bit_alloc(16, 32, 0, 1)) != NULL) {
              bit_blit(win->cursor, 0, 0, 16, 32, BIT_SRC,
                  win->bitmaps[bn - 1], 0, 0);
            } else
              win->cursor = &mouse_arrow;

            SETMOUSEICON(DEFAULT_MOUSE_CURSOR);
          } else {
            /* move mouse */
            mousex = win->esc[0];
            mousey = win->esc[1];
            mousex = BETWEEN(0, mousex, BIT_WIDE(screen) - 1);
            mousey = BETWEEN(0, mousey, BIT_HIGH(screen) - 1);
          }
          if (mouse_was_on)
            MOUSE_ON(screen, mousex, mousey);
        }
        break;
      /*}}}  */
      /*{{{  E_SIZE       -- reshape window: cols, rows*/
      case E_SIZE:
        if (!(win->flags & W_ACTIVE))
          break;
        if (cnt >= 1) {
          int cols = win->esc[cnt - 1];
          int lines = win->esc[cnt];
          int x = win->x0, y = win->y0;

          MOUSE_OFF(screen, mousex, mousey);

          if (cnt >= 3) {
            x = win->esc[0];
            y = win->esc[1];
          }

          if (win != active)
            cursor_off();
          ACTIVE_OFF();
          expose(win);
          shape(x, y,
              cols ? cols * fsizewide + 2 * win->borderwid : 2 * win->borderwid + WIDE,
              lines ? lines * fsizehigh + 2 * win->borderwid : 2 * win->borderwid + HIGH);
          ACTIVE_ON();
          if (!(win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)))
            MOUSE_ON(screen, mousex, mousey);
          done++;
        }
        break;
      /*}}}  */
      /*{{{  E_PUTSNARF   -- put the snarf buffer*/
      case E_PUTSNARF:
        if (snarf)
          Write(win->to_fd, snarf, strlen(snarf));
        break;
      /*}}}  */
      /*{{{  E_GIMME      -- snarf text into input queue*/
      case E_GIMME:
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] > 0 && win->esc[cnt] < MAXSHELL && (win->snarf = malloc(win->esc[cnt] + 1)) != NULL)
          win->flags |= W_TEXT;
        win->code = T_GIMME;
        break;
      /*}}}  */
      /*{{{  E_GMAP       -- read a bitmap from a file*/
      case E_GMAP:
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] > 0 && win->esc[cnt] < MAX_PATH && (win->snarf = malloc(win->esc[cnt] + 1)) != 0)
          win->flags |= W_TEXT;
        win->code = T_GMAP;
        break;
      /*}}}  */
      /*{{{  E_SMAP       -- save a bitmap on a file*/
      case E_SMAP:
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] > 0 && win->esc[cnt] < MAX_PATH && (win->snarf = malloc(win->esc[cnt] + 1)) != 0) {
          win->flags |= W_TEXT;
        }
        win->code = T_SMAP;
        break;
      /*}}}  */
      /*{{{  E_SNARF      -- snarf text into the snarf buffer*/
      case E_SNARF:
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] >= 0 && /*** was just > */
            (win->snarf = malloc(win->esc[cnt] + 1)) != 0)
          win->flags |= W_TEXT;
        win->code = T_YANK;
        break;
      /*}}}  */
      /*{{{  E_STRING     -- write text into the offscreen bitmap*/
      case E_STRING:
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] > 0 && (win->snarf = malloc(win->esc[cnt] + 1)) != 0)
          win->flags |= W_TEXT;
        win->code = T_STRING;
        break;
      /*}}}  */
      /*{{{  E_GRUNCH     -- graphics scrunch mode  (experimental)*/
      case E_GRUNCH: /* graphics scrunch mode  (experimental) */
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] >= 0 && /*** was just > */
            (win->snarf = malloc(win->esc[cnt] + 1)) != 0)
          win->flags |= W_TEXT;
        win->code = T_GRUNCH;
        break;
/*}}}  */
#ifdef XMENU
      /*{{{  E_XMENU      -- extended menu stuff*/
      case E_XMENU:
        /* ^[3X remove menu 3 from window */
        /* ^[3,4X       select item 4 of menu 3 */
        /* ^[1,2,3X     display menu 3 at 1,2 */
        /* ^[1,2,3,4Xhighlight menu 3 item 4 at 1,2 */
        {
          int *p = win->esc;
          struct menu_state *menu;
          switch (cnt) {
          case 0: /* remove menu from display */
            if (p[0] >= 0 && p[0] < MAXMENU && (menu = win->menus[p[0]]))
              menu_remove(menu);
          case 1: /* select active item */
            if (p[0] >= 0 && p[0] < MAXMENU && (menu = win->menus[p[0]]))
              menu->current = p[1];
            break;
          case 2: /* display menu  on window */
            if (p[2] >= 0 && p[2] < MAXMENU && (menu = win->menus[p[2]]))
              menu_setup(menu, window, Scalex(p[0]), Scaley(p[1]), -1);
            break;
          case 3: /* highlight menu item on window */
            if (p[2] >= 0 && p[2] < MAXMENU && (menu = win->menus[p[2]]) && menu->menu) {
              bit_blit(window, Scalex(p[0]) + MENU_BORDER,
                  Scaley(p[1]) + (p[3] - 1) * menu->bar_sizey + MENU_BORDER,
                  menu->bar_sizex, menu->bar_sizey,
                  BIT_NOT(BIT_DST), 0, 0, 0);
            }
            break;
          }
        }
        break;
/*}}}  */
#endif
      /*{{{  E_MENU       -- get a menu*/
      case E_MENU:                 /* get a menu */
      {                            /* should be split into several cases */
        int b = (win->esc[0] < 0); /* which button */
        int n = ABS(win->esc[0]);  /* menu number */

        /* setup menu pointer */

        if (cnt > 2) {
          int parent = n;          /* parent menu # */
          int child = win->esc[2]; /* child menu number */
          int item = win->esc[1];  /* item # of parent */
          int flags = win->esc[3]; /* menu flags */

          if (parent < 0 || parent >= MAXMENU || child >= MAXMENU || win->menus[parent] == NULL)
            break;

          dbgprintf('M', (stderr, "Linking menu %d to parent %d at item %d\n",
                             child, parent, item));

          if (item < 0) /* page link */
            win->menus[parent]->next = child;
          else if (item < win->menus[parent]->count) /* slide lnk */
            menu_setnext(win->menus[parent], item) = child;

          /* menu flags */

          if (flags > 0)
            win->menus[parent]->flags = flags;

          break;
        }

        /* download a menu */

        if (cnt > 0) {
          win->esc[TEXT_COUNT] = 0;
          if (win->menus[n]) {
            menu_destroy(win->menus[n]);
            win->menus[n] = NULL;
            if (win->menu[0] == n)
              win->menu[0] = -1;
            if (win->menu[1] == n)
              win->menu[1] = -1;
          }
          if (win->esc[cnt] > 0 && (win->snarf = malloc(win->esc[cnt] + 1)) != 0) {
            win->flags |= W_TEXT;
            win->code = T_MENU;
          }
          dbgprintf('M', (stderr, "downloading menu %d\n", n));
        }

        /* select menu number */

        else if (n < MAXMENU && win->menus[n]) {
          int last_menu = win->menu[b];

          dbgprintf('M', (stderr, "selecting menu %d on button %d\n", n, b));
          win->menu[b] = n;
          if (last_menu < 0 && button_state == (b ? BUTTON_1 : BUTTON_2))
            go_menu(b);
        } else
          win->menu[b] = -1;
      } break;
      /*}}}  */
      /*{{{  E_EVENT      -- get an event*/
      case E_EVENT:
        switch (cnt) {
        case 2: /* append to an event */
        case 1: /* set an event */
          win->esc[TEXT_COUNT] = 0;
          if (win->esc[cnt] > 0 && (win->snarf = malloc(win->esc[cnt] + 1)) != 0) {
            win->flags |= W_TEXT;
            win->code = T_EVENT;
          }
          break;
        case 0:
          cnt = win->esc[0];
          if (!CHK_EVENT(cnt))
            break;
          EVENT_CLEAR_MASK(win, cnt);
          if (win->events[GET_EVENT(cnt)]) {
            free(win->events[GET_EVENT(cnt)]);
            win->events[GET_EVENT(cnt)] = NULL;
          }
          break;
        }
        break;
      /*}}}  */
      /*{{{  E_SEND       -- send a message*/
      case E_SEND: /* send a message */
        win->esc[TEXT_COUNT] = 0;
        if (win->esc[cnt] > 0 && (win->snarf = malloc(win->esc[cnt] + 1)) != 0) {
          win->flags |= W_TEXT;
          win->code = T_SEND;
        }
        break;
      /*}}}  */
      /*{{{  E_COLORPALETTE   -- set/get color palette entries */
      case E_COLORPALETTE: {
        int i = win->esc[0];
        unsigned int ind, r, g, b, maxi;

        if (cnt == 0) { /* read or allocate palette entry */
          if (i >= 0) {
            getpalette(screen, (unsigned int)i, &r, &g, &b, &maxi);
            sprintf(tbuff, "COLOR %d %u %u %u %u\n", i, r, g, b, maxi);
          } else {
            i = allocate_color(win);
            if (i >= 0)
              sprintf(tbuff, "YOURCOLOR %d\n", i);
            else
              sprintf(tbuff, "\n"); /* none available */
          }
          write(win->to_fd, tbuff, strlen(tbuff));
        } else if (cnt == 1) { /* free some colors */
          free_colors(win, (unsigned int)i, (unsigned int)win->esc[1]);
        } else if (cnt == 3) { /* find a color */
          r = (unsigned int)i;
          g = win->esc[1];
          b = win->esc[2];
          maxi = win->esc[3];
          findcolor(screen, &ind, &r, &g, &b, &maxi);
          sprintf(tbuff, "COLOR %u %u %u %u %u\n", ind, r, g, b, maxi);
          write(win->to_fd, tbuff, strlen(tbuff));
        } else if (cnt == 4) { /* set palette entry */
          r = win->esc[1];
          g = win->esc[2];
          b = win->esc[3];
          maxi = win->esc[4];
          setpalette(screen, (unsigned int)i, r, g, b, maxi);
        }
        break;
      }
      /*}}}  */
      /*{{{  E_BITGET     -- transfer a bitmap from server to client*/
      case E_BITGET: {
        int offset = win->esc[2];
        int which = *win->esc;
        int size = win->esc[1];
        BITMAP *m; /* = win->bitmaps[which-1]; */
        unsigned char *data;

        if (
            cnt > 1
            && which > 0
            && which <= MAXBITMAPS
            && (m = win->bitmaps[which - 1]) != NULL
            && size + offset <= B_SIZE8(BIT_WIDE(m), BIT_HIGH(m), BIT_DEPTH(m))) {
          data = bit_save(m);
          write(win->to_fd, data + offset, size);
          free(data);
          /* write(win->to_fd,BIT_DATA(m)+offset,size); */
        }
        break;
      }
      /*}}}  */
      /*{{{  E_BITVALUE   -- set/get pixel value*/
      case E_BITVALUE: {
        int m = win->esc[0];
        int x = win->esc[1];
        int y = win->esc[2];
        int v = win->esc[3];
        BITMAP *map;

        if ((cnt == 2 || cnt == 3) && m > 0 && m <= MAXBITMAPS) {
          map = win->bitmaps[m - 1];
          if (map) {
            if (cnt == 2) { /* get pixel value */
              sprintf(tbuff, "%d\n", bit_on(map, x, y));
              write(win->to_fd, tbuff, strlen(tbuff));
            } else {
              v &= ~(~0 << BIT_DEPTH(map));
              bit_point(map, x, y, PUTFCOLOR(BIT_SET, v));
            }
          }
        }
        break;
      }
      /*}}}  */
      /*{{{  E_BITCRT     -- create/destroy a bitmap*/
      case E_BITCRT: /* create/destroy a bitmap */
      {
        int bmnbr = win->esc[0];

        switch (cnt) {
        case 0: /* destroy a bitmap */
          if (bmnbr > 0 && bmnbr <= MAXBITMAPS && win->bitmaps[bmnbr - 1]) {
            bit_destroy(win->bitmaps[bmnbr - 1]);
            win->bitmaps[bmnbr - 1] = NULL;
          }
          break;
        case 2: /* create new bitmap - same depth as window */
        case 3: /* " - specify depth, 1->1 bit, otherwise DEPTH */
          if (bmnbr > 0 && bmnbr <= MAXBITMAPS && win->bitmaps[bmnbr - 1]) {
            int w = win->esc[1];
            int h = win->esc[2];

            win->bitmaps[bmnbr - 1] = bit_alloc(Scalex(w),
                Scaley(h),
                0,
                (cnt == 3 && win->esc[3] == 1)
                    ? 1
                    : BIT_DEPTH(win->window));
            dbgprintf('B', (stderr, "%s: created bitmap %d (%d,%d)\r\n",
                               win->tty, bmnbr, w, h));
          }
          break;
        }
      } break;
      /*}}}  */
      /*{{{  E_BITLOAD    -- transfer a bitmap from client to server*/
      case E_BITLOAD:
        if (cnt >= 4) {
          if ((win->snarf = malloc(win->esc[win->esc_cnt])) != 0) {
            win->esc[TEXT_COUNT] = 0;
            win->code = T_BITMAP;
            win->flags |= W_TEXT;
          } else
            fprintf(stderr, "MGR: Can't allocate bitmap!\n");
        }
        break;
      /*}}}  */
      /*{{{  E_SHAPE      -- reshape window, make it active*/
      case E_SHAPE:

        MOUSE_OFF(screen, mousex, mousey);

        ACTIVE_OFF();
        if (win != active) {
          cursor_off();
          expose(win);
        }

        if (cnt >= 3)
          shape(win->esc[cnt - 3], win->esc[cnt - 2],
              win->esc[cnt - 1], win->esc[cnt]);
        else if (cnt == 1)
          shape(win->esc[cnt - 1], win->esc[cnt],
              BIT_WIDE(win->border),
              BIT_HIGH(win->border));

        ACTIVE_ON();
        if (!(win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)))
          MOUSE_ON(screen, mousex, mousey);

        done++;
        break;
      /*}}}  */
      /*{{{  E_BITBLT     -- do a bit blit*/
      case E_BITBLT: /* do a bit blit */
        win_rop(win, window);
        done++;
        break;
      /*}}}  */
      /*{{{  E_CIRCLE     -- Plot a circle (or ellipse)*/
      case E_CIRCLE: /* Plot a circle (or ellipse) */
        circle_plot(win, window);
        break;
      /*}}}  */
      /*{{{  E_LINE       -- Plot a line*/
      case E_LINE:
        win_plot(win, window);
        break;
      /*}}}  */
      /*{{{  E_GO         -- move graphics pointer*/
      case E_GO:
        win_go(win);
        break;
      /*}}}  */
      /*{{{  E_MOVE       -- move to x,y pixels*/
      case E_MOVE: /* move to x,y pixels */
        win->flags &= ~W_SNARFABLE;
        if (Do_clip())
          Set_cliphigh(win->x + win->text.x + fsizewide, win->y + win->text.y);
        if (cnt > 0) {
          win->x = Scalex(*win->esc);
          win->y = Scaley(win->esc[1]);
        } else {
          win->x += Scalex(*win->esc);
        }
        if (win->x + fsizewide > WIDE && !(win->flags & W_NOWRAP))
          win->x = WIDE - fsizewide;
        if (win->y > HIGH)
          win->y = HIGH - fsizehigh;
        if (Do_clip())
          Set_cliplow(win->x + win->text.x, win->y + win->text.y - fsizehigh);
        break;
      /*}}}  */
      /*{{{  E_CUP        -- move to col,row (zero based)*/
      case E_CUP:
        if (cnt < 1)
          break;
        {
          int x = win->esc[cnt - 1] * fsizewide;
          int y = win->esc[cnt] * fsizehigh;
          if (x == BETWEEN(-1, x, T_WIDE - fsizewide) && y == BETWEEN(-1, y, T_HIGH)) {
            if (Do_clip())
              Set_cliphigh(win->x + win->text.x + fsizewide, win->y + win->text.y);
            win->y = y + fsizehigh;
            win->x = x;
            if (Do_clip())
              Set_cliplow(win->x + win->text.x, win->y + win->text.y - fsizehigh);
          }
        }
        break;
      /*}}}  */
      /*{{{  E_VI         -- turn on vi hack*/
      case E_VI: /* turn on vi hack */
        win->flags |= W_VI;
        break;
      /*}}}  */
      /*{{{  E_NOVI       -- turn off vi hack*/
      case E_NOVI: /* turn off vi hack */
        win->flags &= ~W_VI;
        break;
      /*}}}  */
      /*{{{  E_PUSH       -- push environment*/
      case E_PUSH: /* push environment */
        win_push(win, *win->esc);
        break;
      /*}}}  */
      /*{{{  E_POP        -- pop old environment*/
      case E_POP: /* pop old environment */
        MOUSE_OFF(screen, mousex, mousey);
        win_pop(win);
        if (!(win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)))
          MOUSE_ON(screen, mousex, mousey);
        done++;
        break;
      /*}}}  */
      /*{{{  E_TEXTREGION -- setup text region*/
      case E_TEXTREGION: /* setup text region */
        switch (cnt) {
        case 1: /* setup scrolling region (~aka vt100) */
          if (win->esc[0] >= 0 && win->esc[1] >= win->esc[0] && win->esc[1] * fsizehigh < BIT_HIGH(win->window)) {
            win->text.x = 0;
            win->text.wide = BIT_WIDE(win->window);
            win->text.y = fsizehigh * win->esc[0];
            win->text.high = fsizehigh * (1 + win->esc[1]) - win->text.y;
            if (win->y < win->text.y + fsizehigh)
              win->y = win->text.y + fsizehigh;
            if (win->y > win->text.high)
              win->y = win->text.high;
          }
          break;
        case 3: /* set up entire region */
          win->text.wide = Scalex(win->esc[2]);
          win->text.high = Scaley(win->esc[3]);
          win->text.x = Scalex(win->esc[0]);
          win->text.y = Scaley(win->esc[1]);
          if (win->text.high >= fsizehigh * MIN_Y && win->text.wide >= fsizewide * MIN_X) {
            win->x = 0;
            win->y = fsizehigh;
            win->flags &= ~W_SNARFABLE;
            break;
          }
          win->text.x = 0;
          win->text.y = 0;
          win->text.wide = 0;
          win->text.high = 0;
          break;
        case 4: /* set up entire region (use rows, cols) */
          win->text.x = win->esc[0] * fsizewide;
          win->text.y = win->esc[1] * fsizehigh;
          win->text.wide = win->esc[2] * fsizewide;
          win->text.high = win->esc[3] * fsizehigh;
          if (win->text.high >= fsizehigh * MIN_Y && win->text.wide >= fsizewide * MIN_X) {
            win->x = 0;
            win->y = fsizehigh;
            break;
          }
          break;
        case 0: /* clear text region */
          if (win->text.x % fsizewide != 0 || win->text.y % fsizehigh != 0)
            win->flags &= ~W_SNARFABLE;
          win->text.x = 0;
          win->text.y = 0;
          win->text.wide = 0;
          win->text.high = 0;
          break;
        }
        done++;
#ifdef REGION_HACK
        set_size(win); /* emacs trouble when it sets scroll region */
#endif
        break;
      /*}}}  */
      /*{{{  E_SETMODE    -- set a window mode*/
      case E_SETMODE:
        switch (win->esc[0]) {
        case M_STANDOUT:
          standout(win);
          break;
        case M_BACKGROUND: /* enable background writes */
          win->flags |= W_BACKGROUND;
          break;
        case M_NOINPUT: /* disable keyboard input */
          win->flags |= W_NOINPUT;
          break;
        case M_AUTOEXPOSE: /* auto expose upon write */
          win->flags |= W_EXPOSE;
          break;
        case M_WOB: /* set white on black */
          if (win->flags & W_REVERSE)
            break;
          win->flags |= W_REVERSE;
          win->style = SWAPCOLOR(win->style);
          CLEAR(window, BG_OP);
          BORDER(win);
          if (Do_clip())
            Set_all();
          break;
        case M_NOWRAP: /* turn on no-wrap */
          win->flags |= W_NOWRAP;
          break;
        case M_OVERSTRIKE: /* turn on overstrike */
          win->style = PUTOP(win->op, win->style);
          win->flags |= W_OVER;
          break;
        case M_ABS: /* set absolute coordinate mode */
          win->flags |= W_ABSCOORDS;
          break;
        case M_DUPKEY: /* duplicate esc char from keyboard */
          win->flags |= W_DUPKEY;
          if (cnt > 0)
            win->dup = win->esc[1] & 0xff;
          else
            win->dup = DUP_CHAR;
          break;
        case M_NOBUCKEY: /* set no buckey interpretation mode */
          win->flags |= W_NOBUCKEY;
          break;
        case M_CONSOLE: /* redirect console to this device */
          set_console(win, 1);
          break;
#ifndef NOSTACK
        case M_STACK: /* enable event stacking */
          EVENT_SET_MASK(win, EVENT_STACK);
          break;
#endif
        case M_SNARFLINES: /* only cut lines */
          win->flags |= W_SNARFLINES;
          break;
        case M_SNARFTABS: /* change spaces to tabs */
          win->flags |= W_SNARFTABS;
          break;
        case M_SNARFHARD: /* snarf even if errors */
          win->flags |= W_SNARFHARD;
          break;
        case M_NOREPORT: /* don't tell kernel our window size */
          win->flags |= W_NOREPORT;
          break;
        case M_ACTIVATE: /* activate the window */
          if (win == active)
            break;

          MOUSE_OFF(screen, mousex, mousey);

          cursor_off();
          ACTIVE_OFF();
          expose(win);
          ACTIVE_ON();
          cursor_on();
          done++;

          if (!(win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)))
            MOUSE_ON(screen, mousex, mousey);
          break;
        }
        break;
      /*}}}  */
      /*{{{  E_CLEARMODE  -- clear a window mode*/
      case E_CLEARMODE:
        switch (win->esc[0]) {
        case M_STANDOUT:
          standend(win);
          break;
        case M_BACKGROUND: /* don't permit background writes */
          win->flags &= ~W_BACKGROUND;
          break;
        case M_NOINPUT: /* permit keyboard input */
          win->flags &= ~W_NOINPUT;
          break;
        case M_AUTOEXPOSE: /* don't auto expose */
          win->flags &= ~W_EXPOSE;
          break;
        case M_WOB: /* set black-on-white */
          if (!(win->flags & W_REVERSE))
            break;
          win->flags &= ~W_REVERSE;
          win->style = SWAPCOLOR(win->style);
          CLEAR(window, BG_OP);
          BORDER(win);
          if (Do_clip())
            Set_all();
          break;
        case M_NOWRAP: /* turn off no-wrap */
          win->flags &= ~W_NOWRAP;
          break;
        case M_OVERSTRIKE: /* turn off overstrike */
          if (win->flags & W_STANDOUT)
            win->style = PUTOP(BIT_NOT(BIT_SRC), win->style);
          else
            win->style = PUTOP(BIT_SRC, win->style);
          win->flags &= ~W_OVER;
          break;
        case M_ABS: /* set relative coordinate mode */
          win->flags &= ~W_ABSCOORDS;
          break;
        case M_DUPKEY: /* reset keyboard dup-ky mode */
          win->flags &= ~W_DUPKEY;
          break;
        case M_NOBUCKEY: /* reset no buckey interpretation mode */
          win->flags &= ~W_NOBUCKEY;
          break;
        case M_CONSOLE: /* quit console redirection to this device */
          set_console(win, 0);
          break;
#ifndef NOSTACK
        case M_STACK: /* enable event stacking */
          EVENT_CLEAR_MASK(win, EVENT_STACK);
          break;
#endif
        case M_SNARFLINES: /* only cut lines */
          win->flags &= ~W_SNARFLINES;
          break;
        case M_SNARFTABS: /* change spaces to tabs */
          win->flags &= ~W_SNARFTABS;
          break;
        case M_SNARFHARD: /* snarf even if errors */
          win->flags &= ~W_SNARFHARD;
          break;
        case M_NOREPORT: /* don't tell kernel our window size */
          win->flags &= ~W_NOREPORT;
          break;
        case M_ACTIVATE: /* hide the window */
          if (!(win->flags & W_ACTIVE) || next_window == 1)
            break;
          MOUSE_OFF(screen, mousex, mousey);
          if (win != active)
            cursor_off();
          ACTIVE_OFF();
          hide(win);
          ACTIVE_ON();
          if (win != active)
            cursor_on();
          if (!(win->flags & W_ACTIVE && mousein(mousex, mousey, win, 0)))
            MOUSE_ON(screen, mousex, mousey);

          done++;
          break;
        }
        break;
      /*}}}  */
      /*{{{  E_GETINFO    -- send window info back to shell*/
      case E_GETINFO:
        get_info(win, window, text);
        break;
      /*}}}  */
      /*{{{  E_MAKEWIN    -- make or goto a new window*/
      case E_MAKEWIN: /* make or goto a new window */
        MOUSE_OFF(screen, mousex, mousey);
        win_make(win, indx);
        done++;
        break;
      /*}}}  */
      /*{{{  E_HALFWIN    -- make a 1/2 window*/
      case E_HALFWIN: /* make a 1/2 window */
      {
        int *p = win->esc;
        char *tty = NULL;

        if (cnt < 3 || cnt > 4)
          break;
        MOUSE_OFF(screen, mousex, mousey);
        if (win != active)
          cursor_off();
        ACTIVE_OFF();

        switch (cnt) {
        case 4:
          tty = half_window(p[0], p[1], p[2], p[3], p[4]);
          break;
        case 3:
          tty = half_window(p[0], p[1], p[2], p[3], -1);
          break;
        }
        if (win != active)
          cursor_on();
        if (win->flags & W_DUPKEY)
          sprintf(tbuff, "%c %s\n", win->dup, tty ? tty : "");
        else
          sprintf(tbuff, "%s\n", tty ? tty : "");
        if (tty) {
          ACTIVE_ON();
        }
        write(win->to_fd, tbuff, strlen(tbuff));

        done++;
      } break;
      /*}}}  */
      /*{{{  default*/
      default:
        break;
        /*}}}  */
      }
      if (!(win->flags & W_ESCAPE))
        win->flags &= ~W_MINUS;
      break;
    /*}}}  */
    /*{{{  default -- normal character*/
    default: {
      switch (c) {
      /*{{{  ESC -- turn on escape mode*/
      case ESC:
        win->flags |= W_ESCAPE;
        win->flags &= ~(W_MINUS);
        win->esc_cnt = 0;
        win->esc[0] = 0;
        break;
      /*}}}  */
      /*{{{  C_NULL -- null character, ignore*/
      case C_NULL:
        break;
      /*}}}  */
      /*{{{  C_BS -- back space*/
      case C_BS:
        if (Do_clip()) {
          Set_cliphigh(win->x + win->text.x + fsizewide, 0);
        }
        win->x -= fsizewide;
        if (win->x < 0)
          win->x = 0;
        if (Do_clip()) {
          Set_cliplow(win->x + win->text.x, 10000);
        }
        break;
      /*}}}  */
      /*{{{  C_FF -- form feed*/
      case C_FF:
        CLEAR(text, BG_OP);
        win->x = 0;
        win->y = fsizehigh;
        win->flags |= W_SNARFABLE;
        if (Do_clip())
          Set_all();
        done++;
        break;
      /*}}}  */
      /*{{{  C_BELL -- ring the bell*/
      case C_BELL:
        bell_on();
        if (!bell++) {
          CLEAR(win->window, BIT_NOT(BIT_DST));
          CLEAR(win->window, BIT_NOT(BIT_DST));
        }
        break;
      /*}}}  */
      /*{{{  C_TAB -- tab*/
      case C_TAB:
        win->x = ((win->x / fsizewide + 8) & ~7) * fsizewide;
        if (win->x + fsizewide >= T_WIDE) {
          win->x = 0;
          if (win->y + fsizehigh > T_HIGH) {
            scroll(win, text, 0, T_HIGH, fsizehigh, BG_OP);
            done++;
          } else
            win->y += fsizehigh;
        }
        break;
      /*}}}  */
      /*{{{  C_RETURN -- return*/
      case C_RETURN:
        if (Do_clip()) {
          Set_cliphigh(win->x + win->text.x + fsizewide, 0);
          Set_cliplow(win->text.x, 10000);
        }
        win->x = 0;
        break;
      /*}}}  */
      /*{{{  C_NL -- line feed*/
      case C_NL: /* line feed */
        if (win->y + fsizehigh > T_HIGH) {
          scroll(win, text, 0, T_HIGH, fsizehigh, BG_OP);
          done++;
        } else
          win->y += fsizehigh;
        break;
      /*}}}  */
      /*{{{  default -- print a character*/
      default:
        if (win->y > T_HIGH)
          win->y = T_HIGH - fsizehigh;
        PUT_CHAR(text, win->x, win->y, win->font, win->style, offset + c);

        win->x += fsizewide;
        if (win->x + fsizewide > T_WIDE && !(win->flags & W_NOWRAP)) {
          if (Do_clip()) {
            Set_cliphigh(win->x + win->text.x + fsizewide, 0);
            Set_cliplow(win->text.x, 10000);
          }
          win->x = 0;
          win->y += fsizehigh;
          if (win->y > T_HIGH) {
            win->y -= fsizehigh;
            scroll(win, text, 0, T_HIGH, fsizehigh, BG_OP);
            done++;
          }
        }
        break;
        /*}}}  */
      }
      break;
    }
      /*}}}  */
    }
  /*}}}  */

  if (Do_clip()) {
    Set_cliphigh(win->x + win->text.x + fsizewide, win->y + win->text.y);
  }

  cursor_on();

  MOUSE_ON(screen, mousex, mousey);

  /* this is probably wrong */
  if (text != window)
    bit_destroy(text);
  if (sub_window)
    bit_destroy(window);

  if (win->flags & W_BACKGROUND && !(win->flags & W_ACTIVE))
    update(win, &clip);
  return (indx);
}
/*}}}  */
