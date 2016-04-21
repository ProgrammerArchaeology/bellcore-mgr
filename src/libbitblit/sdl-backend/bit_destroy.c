/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "screen.h"

void display_close(BITMAP *bitmap);

/* destroy a bitmap, free up space (might need special code for the display) */

void bit_destroy(BITMAP *bitmap)
{
  printf("bit_destroy(%p)\n", bitmap);

  if (bitmap == NULL)
    return;

  printf("  -> IS_MEMORY? %d\n", IS_MEMORY(bitmap));
  printf("  -> IS_PRIMARY? %d\n", IS_PRIMARY(bitmap));
  printf("  -> IS_SCREEN? %d\n", IS_SCREEN(bitmap));

  if (IS_MEMORY(bitmap) && IS_PRIMARY(bitmap))
  {
#ifdef MOVIE
    log_destroy(bitmap);
#endif
    free(bitmap->data);
    bitmap->data = NULL;
  }
  else if (IS_SCREEN(bitmap) && IS_PRIMARY(bitmap))
  {
#ifdef MOVIE
    log_destroy(bitmap);
#endif
    display_close(bitmap);
  }

  free(bitmap);
  return;
}
