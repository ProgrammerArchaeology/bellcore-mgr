/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include <stdio.h>

#include "screen.h"

int bit_size(int wide, int high, unsigned char depth)
{
  printf("// bit_size(%d, %d, %d)\n", wide, high, depth);
  return BIT_Size(wide, high, depth);
}
