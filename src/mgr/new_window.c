/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* Create a new window */
/*}}}  */
/*{{{  #includes*/
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "clip.h"
#include "defs.h"
#include "menu.h"

#include "proto.h"
#include "Write.h"
#include "border.h"
#include "do_button.h"
#include "font_subs.h"
#include "get_font.h"
#include "get_rect.h"
#include "getshell.h"
#include "icon_server.h"
#include "put_window.h"
#include "subs.h"
#include "update.h"
/*}}}  */

/*{{{  insert_win -- insert a new window into the window list*/
WINDOW *
insert_win(WINDOW *win)
{
  if (win == NULL && (win = (WINDOW *)malloc(sizeof(WINDOW))) == NULL) {
    if (debug)
      fprintf(stderr, "Can't malloc window space\n");
    return (win);
  }

  if (active) {
    win->prev = active->prev;
    active->prev = win;
    win->next = active;
  } else {
    win->prev = win;
    win->next = NULL;
  }
  return (win);
}
/*}}}  */
/*{{{  check_window -- check window size*/
int check_window(int x, int y, int dx, int dy, int fnt)
{
  struct font *curr_font;

  if (dx < 0)
    x += dx, dx = -dx;
  if (dy < 0)
    y += dy, dy = -dy;

  if (x >= BIT_WIDE(screen) || y >= BIT_HIGH(screen))
    return (0);

  if (x + dx >= BIT_WIDE(screen))
    dx = BIT_WIDE(screen) - x;

  if (y + dy >= BIT_HIGH(screen))
    dy = BIT_HIGH(screen) - y;

  curr_font = Get_font(fnt);

  dbgprintf('n', (stderr, "starting: (%d,%d)  %d x %d\r\n", x, y, dx, dy));

  return (dx - SUM_BDR - SUM_BDR >= curr_font->head.wide * MIN_X && dy - SUM_BDR - SUM_BDR >= curr_font->head.high * MIN_Y);
}
/*}}}  */
/*{{{  new_windowset_id -- Look through all the windows for the next available window set id.*/
int next_windowset_id(void)
{
  char list[MAXWIN + 2];
  char *cp;
  WINDOW *win;

  for (cp = list; cp < &list[MAXWIN + 2]; cp++)
    *cp = 0;

  for (win = active; win != NULL; win = win->next)
    list[win->setid] = 1;

  /*	There is no window set ID zero.
      */
  for (cp = list + 1; *cp; cp++)
    ;

  return cp - list;
}
/*}}}  */
/*{{{  setup_window -- initialize window state*/
int setup_window(WINDOW *win, struct font *curr_font, int x, int y, int dx, int dy)
{
  int i;

#ifdef MGR_ALIGN
  alignwin(screen, &x, &dx, SUM_BDR);
#endif

  win->font = curr_font;
  win->x = 0;
  win->y = curr_font->head.high;
  win->esc_cnt = 0;
  win->esc[0] = 0;
  win->flags = W_ACTIVE | init_flags;
#ifdef CUT
  win->flags |= W_SNARFABLE;
#endif
  win->style = BIT_SRC;
  win->curs_type = CS_BLOCK;
  win->x0 = x;
  win->y0 = y;
  win->border = bit_create(screen, x, y, dx, dy);
  win->window = bit_create(win->border, SUM_BDR, SUM_BDR, dx - SUM_BDR * 2, dy - SUM_BDR * 2);

  win->borderwid = SUM_BDR;
  win->outborderwid = OUT_BDR;

  win->text.x = 0;
  win->text.y = 0;
  win->text.wide = 0;
  win->text.high = 0;

  win->bitmap = NULL;
  for (i = 0; i < MAXBITMAPS; i++)
    win->bitmaps
        [i]
        = NULL;

  win->cursor = &mouse_arrow;
  win->save = NULL;
  win->stack = NULL;
  win->main = win;
  win->alt = NULL;
  win->esc_cnt = 0;
  win->esc[0] = 0;
  win->clip_list = NULL;

  for (i = 0; i < MAXMENU; i++)
    win->menus[i] = NULL;

  win->menu[0] = win->menu[1] = -1;
  win->event_mask = 0;

  for (i = 0; i < MAXEVENTS; i++)
    win->events
        [i]
        = NULL;

  win->snarf = NULL;
  win->gx = 0;
  win->gy = 0;
  win->op = BIT_OR;
  win->max = 0;
  win->current = 0;
  strcpy(win->tty, last_tty());
  win->num = 0;
  clip_bad(win); /* invalidate clip lists */
  return (win->border && win->window);
}
/*}}}  */
/*{{{  make_window -- draw the window on the screen*/
int make_window(BITMAP *screen, int x, int y, int dx, int dy, int fnt, char *start)
{
  WINDOW *win = active;
  struct font *curr_font;

  if (dx < 0)
    x += dx, dx = -dx;
  if (dy < 0)
    y += dy, dy = -dy;

  if (x < 0)
    x = 0;

  if (x + dx >= BIT_WIDE(screen))
    dx = BIT_WIDE(screen) - x;

  if (y + dy >= BIT_HIGH(screen))
    dy = BIT_HIGH(screen) - y;

  curr_font = Get_font(fnt);
  if (curr_font == font) {
    dbgprintf('n', (stderr, "Can't find font %d, using default\r\n", fnt));
  }

  dbgprintf('n', (stderr, "starting window: (%d,%d)  %d x %d font (%d,%d)\r\n",
                     x, y, dx, dy, curr_font->head.wide, curr_font->head.high));
  dbgprintf('n', (stderr, "min size: %d x %d\r\n",
                     SUM_BDR + SUM_BDR + curr_font->head.wide * MIN_X,
                     SUM_BDR + SUM_BDR + curr_font->head.high * MIN_Y));

  if (dx < SUM_BDR + SUM_BDR + curr_font->head.wide * MIN_X || dy < SUM_BDR + SUM_BDR + curr_font->head.high * MIN_Y)
    return (-1);

  dbgprintf('n', (stderr, "adjusted to: (%d,%d)  %d x %d\r\n", x, y, dx, dy));

  if (!setup_window(win, curr_font, x, y, dx, dy)) {
    fprintf(stderr, "Out of memory for window creation -- bye!\n");
    quit();
  }

  next_window++;

  /* make the window */

  set_covered(win);
  border(win, BORDER_THIN);
  CLEAR(win->window, PUTOP(BIT_CLR, win->style));

  SETMOUSEICON(DEFAULT_MOUSE_CURSOR); /* because active win chg */

/* set up file descriptor modes */

#ifndef FNDELAY
  if (fcntl(win->from_fd, F_SETFL, fcntl(win->from_fd, F_GETFL, 0) | O_NDELAY) == -1)
#else
  if (fcntl(win->from_fd, F_SETFL, fcntl(win->from_fd, F_GETFL, 0) | FNDELAY) == -1)
#endif
    fprintf(stderr, "%s: fcntl failed for fd %d\n", win->tty, win->from_fd);

  FD_SET(win->to_fd, &mask);
  set_size(win);

  /* send initial string (if any) */

  if (start && *start) {
    dbgprintf('n', (stderr, "Sending initial string: [%s]\n", start));
    Write(win->to_fd, start, strlen(start));
  }
  return (0);
}
/*}}}  */
/*{{{  create_window -- create a new window given coords*/
int create_window(int x, int y, int dx, int dy, int font_num, char **argv)
{
  WINDOW *win;

  if (next_window >= MAXWIN)
    return (-1);
  if (check_window(x, y, dx, dy, font_num) == 0)
    return (-1);

  /* alloc window space */

  if ((win = (WINDOW *)malloc(sizeof(WINDOW))) == NULL) {
    fprintf(stderr, "Can't malloc window space\n");
    return (-1);
  }

  if ((win->pid = get_command(argv, &win->from_fd)) < 0) {
    free(win);
    fprintf(stderr, "mgr: Can't get a pty\n");
    return (-1);
  }
  win->to_fd = win->from_fd;
  win->setid = next_windowset_id();

  active = insert_win(win);

  make_window(screen, x, y, dx, dy, font_num, "");
  return (0);
}
/*}}}  */
/*{{{  half_window -- create a new window given coords, with only 1/2 a ptty*/
char *
half_window(int x, int y, int dx, int dy, int font_num)
{
  WINDOW *win;
  char *tty;

  if (next_window >= MAXWIN)
    return (NULL);
  if (check_window(x, y, dx, dy, font_num) == 0)
    return (NULL);

  /* alloc window space */

  if ((win = (WINDOW *)malloc(sizeof(WINDOW))) == NULL) {
    fprintf(stderr, "Can't malloc window space\n");
    return (NULL);
  }

  if ((tty = half_open(&win->from_fd)) == NULL) {
    free(win);
    fprintf(stderr, "Can't get a pty\n");
    return (NULL);
  }
  win->to_fd = win->from_fd;
  win->setid = next_windowset_id();

  active = insert_win(win);

  make_window(screen, x, y, dx, dy, font_num, "");
  win->pid = 1; /* wont get killed */
  win->flags |= W_NOKILL;

  return (tty);
}
/*}}}  */
/*{{{  new_window -- sweep out a new window*/
void new_window(void)
{
  int dx = 16, dy = 16;

  if (next_window >= MAXWIN)
    return;
  SETMOUSEICON(&mouse_box);
  move_mouse(screen, mouse, &mousex, &mousey, 0);
  SETMOUSEICON(&mouse_arrow);
  get_rect(screen, mouse, mousex, mousey, &dx, &dy, 0);
  do_button(0);

  (void)create_window(mousex, mousey, dx, dy, -1, 0);
}
/*}}}  */
