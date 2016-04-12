/*                        Copyright (c) 1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/*	$Header: pixel.c,v 1.1 89/05/10 18:21:32 sau Locked $
	$Source: /usr/u/sau/mgr/src/dec/RCS/pixel.c,v $
*/
static char	RCSid_[] = "$Source: /usr/u/sau/mgr/src/dec/RCS/pixel.c,v $$Revision: 1.1 $";

/*
 * set/clear/ or invert a pixel (Portable version) (SAU) 
 * return current bit setting (i.e. may be used to replace bit_on)
 */

#include "bitmap.h"

mem_point1(map,x,y,op)
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

	op = GETFCOLOR(op) ? op : rop_invert(op);		/* swap colors */
	x += map->x0;
	y += map->y0;
   base = y * BIT_LINE(map) + (x>>LOGBITS) + (map->data);
   bit = GETLSB(MSB,(x & BITS));

#ifdef MOVIE
	log_point(map,x,y,op);	/* should do ellipses/circles as primitives too */
#endif
  
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
