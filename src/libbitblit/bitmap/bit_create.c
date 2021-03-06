/*{{{}}}*/
/*{{{  #includes*/
#include <stdlib.h>
#include <mgr/bitblit.h>
#include <mgr/share.h>
/*}}}  */

/*{{{  bit_create -- create a bitmap as a sub-rectangle of another bitmap*/
BITMAP *
bit_create(BITMAP *map, int x, int y, int wide, int high)
{
  BITMAP *result;

  if (x + wide > map->wide)
    wide = map->wide - x;
  if (y + high > map->high)
    high = map->high - y;
  if (wide < 1 || high < 1)
    return NULL;

  if ((result = (BITMAP *)malloc(sizeof(BITMAP))) == NULL)
    return NULL;

  result->data = map->data;
  result->x0 = map->x0 + x;
  result->y0 = map->y0 + y;
  result->wide = wide;
  result->high = high;
  result->depth = map->depth;
  result->primary = map->primary;
  result->cache = NULL;
  result->color = 0;
#ifdef MOVIE
  log_create(map);
#endif
  result->id = map->id;
  result->type = map->type;
  result->deviceinfo = map->deviceinfo;
  return (result);
}
/*}}}  */
