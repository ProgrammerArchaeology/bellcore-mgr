/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "screen.h"

/*	Return "true" (one) if the bit at the given x,y position
	is set in the given bitmap.
	Return "false" (zero) if that bit is not set or if the x,y is outside
	the bitmap.
*/

#ifndef FB_AD /* for NEED_ADJUST code */
#define FB_AD(bp,pp) (pp)
#endif

int bit_on( bp, x, y ) register BITMAP	*bp; int x, y;
{
	register int	mask = 1 << (7 - x % 8);

	register DATA	*ip;

	if( x < 0 || x >= BIT_WIDE(bp) || y < 0 ||  y >= BIT_HIGH(bp) )
		return  0;
	ip = BIT_DATA( bp) + y * BIT_LINE(bp) + (x >> 3);
	return  (*FB_AD(bp,ip) & mask) != 0;
}
