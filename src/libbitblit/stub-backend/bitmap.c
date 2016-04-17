/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/*  generic bitblit code routines*/

#include <stdlib.h>
#include "bitmap.h"

/* open the display */

BITMAP *
bit_open(
    char *name /* name of frame buffer */
    )
{
  BITMAP *result;

  if ((result = (BITMAP *)malloc(sizeof(BITMAP))) == (BITMAP *)0)
    return (BIT_NULL);

  /* do what you need to do here yo initialize the display */

  result->primary = result;
  result->data = 0;
  result->x0 = 0,
  result->y0 = 0,
  result->wide = 1000;
  result->high = 900;
  result->type = _SCREEN;
  return (result);
}

/* destroy a bitmap, free up space (might need special code for the display) */

int bit_destroy(BITMAP *bitmap)
{
  if (bitmap == (BITMAP *)0)
    return (-1);
  if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
    free(bitmap->data);
  free(bitmap);
  return (0);
}
