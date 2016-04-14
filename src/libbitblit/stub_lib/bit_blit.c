/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*{{{  #includes*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "screen.h"
/*}}}  */
/*{{{  #defines*/
#define dprintf	if(bit_debug)fprintf
/*}}}  */

/*{{{  bit_blit -- map bit_blits into mem_rops, caching 8 bit images as needed*/
void
bit_blit(dst_map,x_dst,y_dst,wide,high,op,src_map,x_src,y_src)
BITMAP *dst_map;				/* source bitmap */
BITMAP *src_map;				/* destination bitmap */
int x_dst,y_dst;				/* destination coords */
int x_src,y_src;				/* source coords */
int wide,high;					/* bitmap size */
int op;							/* bitmap function */
	{
  }
/*}}}  */

BITMAP *
bit_expand(map,fg,bg)
BITMAP *map;		/* bitmap to expand */
int fg,bg;			/* foreground and background colors */
	{
    return NULL;
	}
/*}}}  */
/*{{{  bit_shrink -- shrink an 8-bit bitmap into a 1 bit bitmap*/
/* shrink an 8-bit bitmap into a 1 bit bitmap */
/* only works for primary bitmaps for now */
/* assumes 32 bit data, 8 bits per pixel */

BITMAP *
bit_shrink(src_map,bg_color)
BITMAP *src_map;	/* bitmap to shrink  - must be a primary bitmap */
int bg_color;		/* color to use as background - all else is on! */
	{
    return NULL;
	}
/*}}}  */
