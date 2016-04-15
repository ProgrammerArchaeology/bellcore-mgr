/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include "bitmap.h"

/*	Return "true" (one) if the bit at the given x,y position
	is set in the given bitmap.
	Return "false" (zero) if that bit is not set or if the x,y is outside
	the bitmap.
*/
#define	BITSPERDATA	(sizeof(int) * 8)

int
bit_on( bp, x, y )
register BITMAP	*bp;
int		x, y;
{
	register int	mask = 1 << (BITSPERDATA - ( x%BITSPERDATA + 1));
	register int	*ip;

	if( x >= BIT_WIDE(bp)  ||  y >= BIT_HIGH(bp) )
		return  0;
	ip = BIT_DATA( bp ) +
		(x/BITSPERDATA + y*BIT_Size( BIT_WIDE(bp), 1, 1 )/sizeof(int));
	return  (*ip & mask) != 0;
}
