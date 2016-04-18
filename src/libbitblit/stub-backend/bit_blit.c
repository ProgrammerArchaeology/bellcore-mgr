/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*{{{  #includes*/
#include <stdlib.h>
#include <string.h>
#include "screen.h"
/*}}}  */

/*{{{  bit_blit -- map bit_blits into mem_rops, caching 8 bit images as needed*/
void bit_blit(
    BITMAP *dst_map,      /* destination bitmap */
    int x_dst, int y_dst, /* destination coords */
    int wide, int high,   /* bitmap size */
    int op,               /* bitmap function */
    BITMAP *src_map,      /* source bitmap */
    int x_src, int y_src  /* source coords */
    )
{
}
/*}}}  */

BITMAP *bit_expand(
    BITMAP *map,   /* bitmap to expand */
    int fg, int bg /* foreground and background colors */
    )
{
  return NULL;
}
/*}}}  */
/*{{{  bit_shrink -- shrink an 8-bit bitmap into a 1 bit bitmap*/
/* shrink an 8-bit bitmap into a 1 bit bitmap */
/* only works for primary bitmaps for now */
/* assumes 32 bit data, 8 bits per pixel */

BITMAP *bit_shrink(
    BITMAP *src_map, /* bitmap to shrink  - must be a primary bitmap */
    int bg_color     /* color to use as background - all else is on! */
    )
{
  return NULL;
}
/*}}}  */
