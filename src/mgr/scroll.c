/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*****************************************************************************
 *	scroll a bitmap
 */
/*}}}  */
/*{{{  #includes*/
#include <stdio.h>

#include <mgr/bitblit.h>

#include "clip.h"
#include "defs.h"
/*}}}  */

/*{{{  scroll -- scroll a bitmap*/
void scroll(
    WINDOW *win, /* window to scroll */
    BITMAP *map, /* bitmap in window to scroll */
    int start,
    int end,
    int delta,
    int op /* starting line, ending line, # of lines */
    )
{
  int ems = end - start;
  if (delta > 0) {
    if (end - start > delta)
#ifdef MGR_ALIGN
      if (win->window == map) {
        dbgprintf('F', (stderr, "fast scroll %s\r\n", win->tty));
        /* special high-speed byte-aligned scroller */

        bit_bytescroll(map, BIT_X(map), BIT_Y(map) + start,
            BIT_WIDE(map) + win->borderwid, end - start, delta);
      } else
#endif /* MGR_ALIGN */
        bit_blit(map, 0, start, BIT_WIDE(map), ems - delta, BIT_SRC, map, 0, start + delta);
    bit_blit(map, 0, end - delta, BIT_WIDE(map), delta, op, 0, 0, 0);
  }

  else if (delta < 0) {
    if (ems + delta > 0)
      bit_blit(map, 0, start - delta, BIT_WIDE(map), ems + delta,
          BIT_SRC, map, 0, start);
    bit_blit(map, 0, start, BIT_WIDE(map), -delta, op, NULL_DATA, 0, 0);
  }

  if (Do_clip())
    Set_clip(win->text.x,
        win->text.y + start,
        win->text.x + BIT_WIDE(map),
        win->text.y + BIT_HIGH(map));
}
/*}}}  */
