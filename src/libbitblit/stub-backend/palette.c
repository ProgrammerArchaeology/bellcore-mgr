/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "screen.h"

/* returns the color index in the color lookup table of the foreground */
unsigned int fg_color_idx(void) { return 63; }

void setpalette(BITMAP *bp, unsigned int index, unsigned int red,
    unsigned int green, unsigned int blue, unsigned int maxi)
{
}

void getpalette(BITMAP *bp, unsigned int index, unsigned int *red,
    unsigned int *green, unsigned int *blue, unsigned int *maxi)
{
}