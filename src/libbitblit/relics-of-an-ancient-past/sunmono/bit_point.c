/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*
 * set/clear/ or invert a pixel (Portable version) (SAU) 
 * return current bit setting (i.e. may be used to replace bit_on)
 */

#include "screen.h"

int bit_point(map,x,y,op)
register BITMAP *map;
register int x,y;
int op;
   {
   register int bit;							/* dst bit */
	register DATA *base;							/* dst word */

   /* clipping */

#ifndef NOCLIP
   if (x<0 || x>BIT_WIDE(map) || y<0 || y>BIT_HIGH(map))
      return(-1);
#endif

#ifdef INVERT
	/* invert all raster ops */

	op = op_invert[15&op];
#endif

	x += map->x0;
	y += map->y0;
   base = y * BIT_LINE(map) + (x>>LOGBITS) + (map->data);
   bit = GETLSB(MSB,(x & BITS));
  
   switch(OPCODE(op)) {
		case OPCODE(SRC):
		case OPCODE(SRC | DST):
		case OPCODE(SRC | ~DST):
		case OPCODE(~0):
			*base |= bit;
         break;
		case OPCODE(~SRC):
		case OPCODE(~(SRC|DST)):
		case OPCODE(DST & ~SRC):
		case OPCODE(0):
			*base &= ~bit;
         break;
		case OPCODE(SRC ^ DST):
		case OPCODE(~DST):
		case OPCODE(SRC & ~DST):
		case OPCODE(~(SRC&DST)):
			*base ^= bit;
         break;
      }
   return(*base&bit);
   }
