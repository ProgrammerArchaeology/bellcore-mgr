/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*  stub bitblit code */

#include "bitmap.h"

/*
 *  General memory-to-memory rasterop
 */

int
mem_rop(dest, dx, dy, width, height, func, source, sx, sy)
int sx, sy, dx, dy;		/* properly clipped source and dest */
int width, height;		/* rectangle to be transferred */
BITMAP *source, *dest;		/* bit map pointers */
int func;			/* rasterop function */
   {
    return -1;
   }
