/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* see if two windows intersect */
/*}}}  */

/*{{{  #includes*/
#include <stdio.h>

#include <mgr/bitblit.h>

#include "defs.h"
/*}}}  */

/*{{{  intersect*/
int intersect(WINDOW *win1, WINDOW *win2)
{
  return (!(
      win1->x0 + BIT_WIDE(win1->border) < win2->x0 || win2->x0 + BIT_WIDE(win2->border) < win1->x0 || win1->y0 + BIT_HIGH(win1->border) < win2->y0 || win2->y0 + BIT_HIGH(win2->border) < win1->y0));
}
/*}}}  */
/*{{{  alone -- see if any window intersects any other*/
int alone(WINDOW *check)
{
  WINDOW *win;

  for (win = active; win != NULL; win = win->next)
    if (check != win && intersect(check, win))
      return (0);
  return (1);
}
/*}}}  */
/*{{{  mousein -- see if mouse is in window*/
int mousein(
    int x,
    int y,
    WINDOW *win,
    int how /* how:  0-> intersect   else -> point */
    )
{
  if (how == 0)
    return (!(
        x + 16 < win->x0 || x > win->x0 + BIT_WIDE(win->border) || y + 16 < win->y0 || y > win->y0 + BIT_HIGH(win->border)));
  else
    return (!(
        x < win->x0 || x > win->x0 + BIT_WIDE(win->border) || y < win->y0 || y > win->y0 + BIT_HIGH(win->border)));
}
/*}}}  */
/*{{{  in_text -- see if mouse is in text region*/
int in_text(int x, int y, WINDOW *win)
{
  if (win->text.wide) {
    int x0 = win->x0 + win->text.x;
    int y0 = win->y0 + win->text.y;
    return (!(
        x < x0 || x > x0 + win->text.wide || y < y0 || y > y0 + win->text.high));
  } else
    return (mousein(x, y, win, 1));
}
/*}}}  */
