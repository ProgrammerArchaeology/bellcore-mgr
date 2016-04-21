/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*  stub bitblit code */

#include "screen.h"

/*
 *  General memory-to-memory rasterop
 */

int mem_rop(
    BITMAP *dest, /* bit map pointers */
    int dx,
    int dy, /* properly clipped source and dest */
    int width,
    int height, /* rectangle to be transferred */
    int func,   /* rasterop function */
    BITMAP *source,
    int sx,
    int sy)
{
  return -1;
}
