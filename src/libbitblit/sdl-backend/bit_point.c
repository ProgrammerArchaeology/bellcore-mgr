/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* draw a point  stub */

#include <stdio.h>

#include "screen.h"

int bit_point(BITMAP *map, int dx, int dy, int func)
{
  printf("bit_point(%p, %d, %d, %d)\n", map, dx, dy, func);
  return -1;
}
