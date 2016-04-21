/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
*/

/* re-shape a window */
/*}}}  */
/*{{{  #includes*/
#include <stdio.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "border.h"
#include "do_button.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_rect.h"
#include "icon_server.h"
#include "intersect.h"
#include "put_window.h"
#include "scroll.h"
#include "subs.h"
#include "update.h"
/*}}}  */

/*{{{  shape -- reshape a window to specified dimensions*/
int shape(int x, int y, int dx, int dy)
{
  int sx, sy, w, h;
  WINDOW *win;

  if (dx > 0) {
    sx = x;
    w = dx;
  } else {
    sx = x + dx;
    w = -dx;
  }
  if (dy > 0) {
    sy = y;
    h = dy;
  } else {
    sy = y + dy;
    h = -dy;
  }

  if (sx < 0)
    sx = 0;

  if (sx + w >= BIT_WIDE(screen))
    w = BIT_WIDE(screen) - sx;

  if (sy + h >= BIT_HIGH(screen))
    h = BIT_HIGH(screen) - sy;

  if (w < 2 * active->borderwid + active->font->head.wide * MIN_X || h < 2 * active->borderwid + active->font->head.high * MIN_Y)
    return (-1);

#ifdef MGR_ALIGN
  alignwin(screen, &sx, &w, active->borderwid);
#endif

  /* remove current window position */
  save_win(active);
  erase_win(active->border);
  clip_bad(active); /* invalidate clip lists */

  /* redraw remaining windows */
  repair(active);

  /* adjust window state */
  active->x0 = sx;
  active->y0 = sy;
  bit_destroy(active->window);
  bit_destroy(active->border);
  active->border = bit_create(screen, sx, sy, w, h);
  active->window = bit_create(active->border,
      active->borderwid,
      active->borderwid,
      w - active->borderwid * 2,
      h - active->borderwid * 2);

  for (win = active->next; win != (WINDOW *)0; win = W(next)) {
    if (W(flags) & W_ACTIVE && intersect(active, win))
      save_win(win);
  }

  CLEAR(active->window, PUTOP(BIT_CLR, active->style));

  border(active, BORDER_THIN);
  bit_blit(active->border, 0, 0,
      BIT_WIDE(active->save) - active->borderwid,
      BIT_HIGH(active->save) - active->borderwid,
      BIT_SRC, active->save, 0, 0);

  /* make sure character cursor is in a good spot */
  if (active->x > BIT_WIDE(active->window)) {
    active->x = 0;
    active->y += ((int)(active->font->head.high));
  }
  if (active->y > BIT_HIGH(active->window)) {
#ifdef WIERD
    active->y = BIT_HIGH(active->window);
    scroll(active->window, 0, BIT_HIGH(active->window),
        ((int)(active->font->head.high)), SWAPCOLOR(active->style));
    bit_blit(active->window, 0, BIT_HIGH(active->window) - ((int)(active->font->head.high)),
        BIT_WIDE(active->save), ((int)(active->font->head.high)),
        BIT_SRC, active->save,
        active->borderwid, BIT_HIGH(active->save) - ((int)(active->font->head.high)) - active->borderwid);
#else
    active->y = BIT_HIGH(active->window) - ((int)(active->font->head.high));
#endif
  }

  bit_destroy(active->save);
  active->save = (BITMAP *)0;

  /* invalidate clip lists */
  clip_bad(active);
  un_covered();
  set_size(active);
  return (0);
}
/*}}}  */
/*{{{  shape_window -- reshape a window with the mouse*/
void shape_window(void)
{
  int dx = 16, dy = 16;

  SETMOUSEICON(&mouse_box);
  move_mouse(screen, mouse, &mousex, &mousey, 0);
  SETMOUSEICON(DEFAULT_MOUSE_CURSOR);
  get_rect(screen, mouse, mousex, mousey, &dx, &dy, 0);
  do_button(0);

  /* look for shape event here */
  do_event(EVENT_SHAPE, active, E_MAIN);

  (void)shape(mousex, mousey, dx, dy);
}
/*}}}  */
/*{{{  stretch_window -- stretch a window with the mouse*/
#ifdef STRETCH
void stretch_window(void)
{
  int dx, dy;
  int x0, x1, y0, y1;

  SETMOUSEICON(&mouse_box);
  move_mouse(screen, mouse, &mousex, &mousey, 0);
  SETMOUSEICON(DEFAULT_MOUSE_CURSOR);

  x0 = active->x0;
  y0 = active->y0;
  x1 = x0 + BIT_WIDE(active->border);
  y1 = y0 + BIT_HIGH(active->border);
  if (2 * (mousex - x0) < x1 - x0)
    x0 = x1;
  dx = mousex - x0;
  if (2 * (mousey - y0) < y1 - y0)
    y0 = y1;
  dy = mousey - y0;
  /* x0,y0 is corner farthest from mouse. x0+dx,y0+dx is mouse position */

  get_rect(screen, mouse, x0, y0, &dx, &dy, 0);
  do_button(0);

  /* look for shape event here */
  do_event(EVENT_SHAPE, active, E_MAIN);

  (void)shape(x0, y0, dx, dy);
}
#endif /* STRETCH */
/*}}}  */
