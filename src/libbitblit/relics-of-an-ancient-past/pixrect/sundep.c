/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*	this is missing from pixrect, and it must be a function */

#include "bitmap.h"

int
bit_point(m,x,y,func)
BITMAP *m;
int x,y;
int func;
   {
   bit_line(m,x,y,x,y,func);
   }

int
bit_on( bp, x, y )
register BITMAP	*bp;
int		x, y;
   {
	return pr_get(bp, x, y);
   }
