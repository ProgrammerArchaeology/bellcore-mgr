/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1988 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* update a background window */

/*
 *	   build a clip list
 *	1) find all window edges that intersect with the current window 
 *	2) sort all tops+bottoms then left+rights into increasing order
 *	3) for each rectangular patch, see if it is visible on target window
 *	4) if visible, coellesce patch and keep on a list
 *    update window against clip list
 * 5) run through list, and update each rext that intersects with
 *    the clip region
 */
/*}}}  */
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "clip.h"

#include "intersect.h"
/*}}}  */

/*{{{  #defines*/
#define MAX_COORDS	((MAXWIN+1)  * 5)	/* educated guess (stab in the dark?) */
/*}}}  */

/*{{{  variables*/
struct rect_list {				/* list of visible "patches" of obscured window */
	rectangle rect;				/* clipping rectangle */
	struct rect_list *next;		/* next clipping rectangle */
	};

static int x[MAX_COORDS];		/* left/right edges of covering windows */
static int y[MAX_COORDS];		/* top/bottom edges of covering windows */

static rectangle *got_int(rectangle *r1,rect *r2,int bwid); /* finds intersecting rectangle */
static void in_mouseoff(int x0,int y0,int wide,int high);
static int in_win(WINDOW *win,int x0,int y0,int x1,int y1);
static int cmp(const void *x,const void *y);	/* compare for qsort */

/*}}}  */

/*{{{  gen_list -- generate a clip list*/
static int gen_list(WINDOW *window)
   {

   register WINDOW *win = window;
   register struct rect_list *list, *prev = (struct rect_list *)0;
   register int x_cnt = 2, y_cnt = 2;
   register int i, j;
   int count = 0;
   int skip;		/* covered by another window - skip patch */
   int hold;		/* hold for coellessing */

   /* build arrays of window coordinates: intersecting win's above win */

   x[0] = W(borderwid) + W(x0);
   y[0] = W(borderwid) + W(y0);
   x[1] = W(borderwid) + W(x0) + BIT_WIDE(W(window));
   y[1] = W(borderwid) + W(y0) + BIT_HIGH(W(window));

   for(win = active; win != window; win=W(next)) {
      if (!(in_win(win,x[0],y[0],x[1],y[1])))
         continue;

      if (W(x0) >= x[0] && W(x0) <= x[1])
         x[x_cnt++] = W(x0);

      if (W(y0) >= y[0] && W(y0) <= y[1])
         y[y_cnt++] = W(y0);

      if (W(x0) + BIT_WIDE(W(border)) >= x[0] &&
               W(x0) + BIT_WIDE(W(border)) <= x[1])
         x[x_cnt++] = W(x0) + BIT_WIDE(W(border));

      if (W(y0) + BIT_HIGH(W(border)) >= y[0] &&
               W(y0) + BIT_HIGH(W(border)) <= y[1])
         y[y_cnt++] = W(y0) + BIT_HIGH(W(border));
      
      if (y_cnt >= MAX_COORDS || x_cnt >= MAX_COORDS)
         break;
      }

   /* sort window coordinate lists */

   qsort(x,x_cnt,sizeof(int),cmp);
   qsort(y,y_cnt,sizeof(int),cmp);

   x_cnt--;
   y_cnt--;

   /* build list of covering rectangles */

   for(j=0; j<y_cnt; j++) {

      if (y[j] == y[j+1])	/* avoid zero-height patches */
         continue;

      for(hold=x_cnt,i=0; i<x_cnt; i++) {

         if (x[i] == x[i+1])	/* avoid zero-width patches */
            continue;

			/* see if patch is visible */

         for(skip=0,win=active; win!=window; win=W(next))
            if (in_win(win, x[i], y[j], x[i+1], y[j+1])) {
               skip++;
               break;
               }

			/* visible, add patch to list, or append to previous patch */

         if (!skip)  {
				if (i == hold) {		/* coel. across */
					list->rect.wide += x[i+1] - x[i];
					hold++;
					}
				else {	/* flush held rect */
					count++;		/* only for debugging */
					list = malloc(sizeof(struct rect_list));
					list->rect.x = x[i] - W(x0);
					list->rect.y = y[j] - W(y0);
					list->rect.wide = x[i+1] - x[i];
					list->rect.high = y[j+1] - y[j];
					list -> next = NULL;
					if (prev)
						prev -> next = list;
					if (!W(clip_list))	/* set initial rectangle */
						W(clip_list) = (char *) list;
					prev = list;
					hold = i+1;				/* next 'i' to check for coell. */
					}
            }
         }
      }

/* look at rect list	DEBUG code, commented out!

	for(list=(struct rect_list *) W(clip_list);list;list = list->next) {
		int	x = list->rect.x,
			y = list->rect.y,
			wide = list->rect.wide,
			high = list->rect.high;
		in_mouseoff( x, y, wide, high );
		bit_blit(W(border), x, y, wide, high, BIT_NOT(BIT_DST),0L,0,0);
		dbgprintf('U',(stderr,"  Rect %d,%d  %dx%d\n", x, y, wide, high ));
		getchar();
		bit_blit(W(border), x, y, wide, high, BIT_NOT(BIT_DST),0L,0,0);
		MOUSE_ON(screen,mousex,mousey);
		}
DEBUG code, commented out! */

	dbgprintf('U',(stderr,"%s: Built clip list (%d)\r\n",W(tty),count));

	return(0);	/* I'll think of something */
   }
/*}}}  */
/*{{{  do_update -- update obscured window*/
static void do_update(WINDOW *win, rect *clipp)
	{
   register struct rect_list *list;	/* list of rectangle to clip to */
   register rectangle *got;		/* intersecting region */

	dbgprintf('U',(stderr,"Updating background window to %d,%d => %d,%d\r\n",
		      clipp->x1,clipp->y1,clipp->x2,clipp->y2));

	for(list=(struct rect_list *)W(clip_list);list;list = list->next) {
		if ((got = got_int(&(list->rect),clipp,W(borderwid)))) {
			register int	x = got->x,
					y = got->y,
					wide = got->wide,
					high = got->high;
			in_mouseoff( x + W(x0), y + W(y0), wide, high );
			bit_blit(W(border),x,y,wide,high,BIT_SRC,W(save),x,y);
			MOUSE_ON(screen,mousex,mousey);
			}
		}
	}
/*}}}  */
/*{{{  rectangle -- find the intersection of 2 rectangles*/
static rectangle *got_int(rectangle *r1, /* rect 1 */
			  rect *r2,      /* other rect (should both be same struct) */
			  int bwid)      /* border width */
	{
	static rectangle result;

	result.x = Max(r1->x,r2->x1+bwid);
	result.y = Max(r1->y,r2->y1+bwid);
	result.wide = Min(r1->x + r1->wide, r2->x2+bwid) - result.x;
	result.high = Min(r1->y + r1->high, r2->y2+bwid) - result.y;

	if (result.wide > 0 && result.high > 0 )
      return(&result);
	else
		return((rectangle *) 0);
	}
/*}}}  */
/*{{{  in_mouseoff -- see if mouse in rectangle, if so turn the mouse off*/
static void in_mouseoff(x0,y0,wide,high)
register int x0,y0,wide,high;
   {
   if( !( x0 > mousex+16 || y0 > mousey+16 ||
        x0+wide < mousex || y0+high < mousey))
	MOUSE_OFF(screen,mousex,mousey);
   }
/*}}}  */
/*{{{  in_win -- see if rectangle in window*/
static int
in_win(win,x0,y0,x1,y1)
register WINDOW *win;
register x0,y0,x1,y1;
   {
   return(
      W(x0) + BIT_WIDE(W(border)) <= x0 ||
      x1 <= W(x0) ||
      W(y0) + BIT_HIGH(W(border)) <= y0 ||
      y1 <= W(y0)
   ?  0  :  1);
   }
/*}}}  */
/*{{{  cmp -- compare for sort*/
static int
cmp(x,y)
const void *x, *y;
   {
   return( *(int *)x - *(int *)y);
   }
/*}}}  */

/*{{{  zap_cliplist -- free window's clip list*/
void zap_cliplist(WINDOW *win)
	{
	register struct rect_list *list = (struct rect_list *) W(clip_list);
	register struct rect_list *next;

	dbgprintf('U',(stderr,"Zapping clip list\r\n"));
	for(;list;list=next) {
		next = list->next;
		free(list);
		}	
	W(clip_list) = NULL;
	}
/*}}}  */
/*{{{  update -- update an obscured window*/
void update(WINDOW *win, rect *clipp)
	{
	/* generate clip list */

	if (!(W(flags)&W_CLIPDONE)) {
		if (W(clip_list) != NULL) /* free old list (we could reuse it) */
			zap_cliplist(win);
		gen_list(win);
		W(flags) |= W_CLIPDONE;
		}

	/* update the window */

   do_update(win,clipp);			/* do the update */
	}
/*}}}  */
/*{{{  clip_bad -- invalidate clip list for all windows affected by 'window'*/
void clip_bad(window)
register WINDOW *window;		/* this window has changed */
	{
	register WINDOW *win;		/* working window */

	/* invalidate all intersecting window clip lists below this one */

	window->flags &= ~W_CLIPDONE;		/* invalidate clip list */
   for(win=window->next;win != (WINDOW *) 0;win=W(next))
		if (intersect(win,window)) 
			W(flags) &= ~W_CLIPDONE;		/* invalidate clip list */
	}
/*}}}  */
