/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* push - pop window environment */
/*}}}  */
/*{{{  #includes*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "defs.h"
#include "event.h"
#include "menu.h"

#include "border.h"
#include "destroy.h"
#include "do_event.h"
#include "font_subs.h"
#include "get_menus.h"
#include "icon_server.h"
#include "print.h"
#include "put_window.h"
#include "shape.h"
#include "subs.h"
/*}}}  */

/*{{{  win_push -- push a window on the environment stack*/
int win_push(
    WINDOW *win, /* window to push */
    int level    /* what things to push */
    )
{
  WINDOW *stack; /* pushed window goes here */
  int i, j;

  if ((stack = (WINDOW *)malloc(sizeof(WINDOW))) == NULL)
    return -1;

  if (level == 0)
    level = P_DEFAULT;

  dbgprintf('P', (stderr, "%s Stacking %s\n", win->tty, print_stack(level)));

  /* setup stacked window */

  (void)memcpy(stack, win, sizeof(WINDOW));
  win->stack = stack;

  for (j = 0; j < MAXMENU; j++)
    stack->menus[j] = NULL;
  for (j = 0; j < MAXEVENTS; j++)
    stack->events[j] = NULL;
  for (j = 0; j < MAXBITMAPS; j++)
    stack->bitmaps[j] = NULL;
  stack->save = NULL;
  stack->clip_list = NULL;

  /* setup each pushed item */

  for (i = 1; i != P_MAX; i <<= 1)
    switch (level & i) {
    case P_MENU: /* save the menus */
      dbgprintf('P', (stderr, "  menus "));
      for (j = 0; j < MAXMENU; j++)
        if (win->menus[j] && (level & P_CLEAR)) {
          stack->menus[j] = win->menus[j];
          win->menus[j] = NULL;
          dbgprintf('P', (stderr, "%d ", j));
        } else if (win->menus[j]) {
          stack->menus[j] = menu_copy(win->menus[j]);
          dbgprintf('P', (stderr, "%d ", j));
        }
      dbgprintf('P', (stderr, "\n"));
      break;
    case P_EVENT: /* save the events */

      dbgprintf('P', (stderr, "  events "));
      if (IS_EVENT(win, EVENT_STACK))
        EVENT_SET_MASK(win, EVENT_STFLAG);

      if (level & P_CLEAR)
        win->event_mask = IS_EVENT(win, EVENT_STFLAG);
      else
        EVENT_CLEAR_MASK(win, EVENT_STACK);

      for (j = 0; j < MAXEVENTS; j++)
        if (win->events[j] && (level & P_CLEAR)) {
          stack->events[j] = win->events[j];
          win->events[j] = NULL;
          dbgprintf('P', (stderr, "%d ", j));
        } else if (win->events[j]) {
          stack->events[j] = strcpy(malloc(strlen(win->events[j]) + 1), win->events[j]);
          dbgprintf('P', (stderr, "%d ", j));
        }
      dbgprintf('P', (stderr, "\n"));
      break;
    case P_CURSOR: /* restore the cursor style */
      if (level & P_CLEAR)
        win->curs_type = CS_BLOCK;
      break;
    case P_BITMAP: /* save the bitmaps */
      dbgprintf('P', (stderr, "  bitmaps "));
      for (j = 0; j < MAXBITMAPS; j++)
        if (win->bitmaps[j] && level & P_CLEAR) {
          stack->bitmaps[j] = win->bitmaps[j];
          win->bitmaps[j] = NULL;
          dbgprintf('P', (stderr, "%d ", j));
        } else if (win->bitmaps[j]) {
          stack->bitmaps[j] = bit_alloc(BIT_WIDE(win->bitmaps[j]),
              BIT_HIGH(win->bitmaps[j]), NULL_DATA,
              BIT_DEPTH(win->bitmaps[j]));
          bit_blit(stack->bitmaps[j], 0, 0, BIT_WIDE(win->bitmaps[j]),
              BIT_HIGH(win->bitmaps[j]), BIT_SRC, win->bitmaps[j], 0, 0);
          dbgprintf('P', (stderr, "%d ", j));
        }
      dbgprintf('P', (stderr, "\n"));
      break;
    case P_WINDOW: /* save the bit image */
      dbgprintf('P', (stderr, "  window\n"));
      stack->save = bit_alloc(BIT_WIDE(win->border), BIT_HIGH(win->border),
          NULL_DATA, BIT_DEPTH(win->window));
      if (win->save && !(win->flags & W_ACTIVE))
        bit_blit(stack->save, 0, 0, BIT_WIDE(win->save), BIT_HIGH(win->save),
            BIT_SRC, win->save, 0, 0);
      else
        bit_blit(stack->save, 0, 0, BIT_WIDE(win->border), BIT_HIGH(win->border),
            BIT_SRC, win->border, 0, 0);
      break;
    case P_POSITION: /* save the window position */
      dbgprintf('P', (stderr, "  position\n"));
      stack->esc[1] = BIT_WIDE(win->border);
      stack->esc[2] = BIT_HIGH(win->border);
      break;
    case P_TEXT: /* save text region */
      if (level & P_CLEAR) {
        win->text.x = 0;
        win->text.y = 0;
        win->text.wide = 0;
        win->text.high = 0;
        set_size(win);
      }
      break;
    case P_MOUSE: /* save mouse position */
      dbgprintf('P', (stderr, "  mouse\n"));
      stack->esc[3] = mousex;
      stack->esc[4] = mousey;
      if (level & P_CLEAR) {
        stack->cursor = win->cursor;
        win->cursor = &mouse_arrow;
      } else if (IS_STATIC(win->cursor)) {
        stack->cursor = win->cursor;
      } else {
        /* user-defined cursor shape */
        stack->cursor = bit_alloc(BIT_WIDE(win->cursor), BIT_HIGH(win->cursor),
            NULL_DATA, BIT_DEPTH(win->cursor));
        bit_blit(stack->cursor, 0, 0, BIT_WIDE(win->cursor), BIT_HIGH(win->cursor),
            BIT_SRC, win->cursor, 0, 0);
      }
      break;
    case P_FLAGS: /* save window flags  */
      if (level & P_CLEAR) {
        win->flags &= ~W_SAVE;
        win->flags |= W_BACKGROUND;
        win->style = PUTOP(BIT_SRC, win->style);
        border(win, win == active ? BORDER_FAT : BORDER_THIN);
        win->dup = '\0'; /* clear the dupkey mode */
      }
      break;
    case P_COLOR: /* save the colors  */
      if (level & P_CLEAR) {
        win->style = PUTOP(win->style, BIT_SRC);
        win->op = PUTOP(win->op, BIT_OR);
      }
      break;
    case P_BITOP: /* save the bitblit ops  */
      if (level & P_CLEAR) {
        win->style = PUTOP(BIT_SRC, win->style);
        win->op = PUTOP(BIT_OR, win->op);
      }
      break;
    }

  stack->code = level;
  stack->window = NULL;
  stack->border = NULL;
  stack->snarf = NULL;
  stack->bitmap = NULL;
  return level;
}
/*}}}  */
/*{{{  win_pop -- pop the window stack */
int win_pop(
    WINDOW *win /* window to pop to */
    )
{
  int i, j;
  WINDOW *stack = win->stack; /* window to pop from */

  if (stack == NULL) {
    dbgprintf('P', (stderr, "  No environment to pop\n"));
    return -1;
  }

  dbgprintf('P', (stderr, "%s popping %s\n", win->tty, print_stack(stack->code)));

  /* pop each item stacked */

  for (i = 1; i != P_MAX; i <<= 1)
    switch (stack->code & i) {
    case P_MENU: /* restore the menus */
      dbgprintf('P', (stderr, "  menus "));
      win->menu[0] = stack->menu[0];
      win->menu[1] = stack->menu[1];
      for (j = 0; j < MAXMENU; j++) {
        if (win->menus[j]) {
          dbgprintf('P', (stderr, "d(%d) ", j));
          menu_destroy(win->menus[j]);
        }
        if (stack->menus[j]) {
          dbgprintf('P', (stderr, "r(%d) ", j));
          win->menus[j] = stack->menus[j];
          stack->menus[j] = NULL;
        } else
          win->menus[j] = NULL;
      }
      dbgprintf('P', (stderr, "\n"));
      break;
    case P_EVENT: /* restore the events */

      dbgprintf('P', (stderr, "  events "));
      for (j = 0; j < MAXEVENTS; j++) {
        if (win->events[j]) {
          dbgprintf('P', (stderr, "d(%d) ", j));
          free(win->events[j]);
        }
        win->events[j] = stack->events[j];
        stack->events[j] = NULL;
      }
      win->event_mask = stack->event_mask;
      dbgprintf('P', (stderr, "\n"));
      break;
    case P_CURSOR: /* restore the cursor position */
      win->x = stack->x;
      win->y = stack->y;
      win->gx = stack->gx;
      win->gy = stack->gy;
      win->curs_type = stack->curs_type;
      break;
    case P_BITMAP: /* restore the bitmaps */
      for (j = 0; j < MAXBITMAPS; j++) {
        if (win->bitmaps[j])
          bit_destroy(win->bitmaps[j]);
        win->bitmaps[j] = stack->bitmaps[j];
        stack->bitmaps[j] = NULL;
      }
      dbgprintf('P', (stderr, "  bitmaps\n"));
      break;
    case P_FONT: /* restore font */
      win->font = stack->font;
      break;
    case P_TEXT: /* restore text region */
      win->text = stack->text;
      set_size(win);
      break;
    case P_POSITION: /* restore the window position */
      if (win != active)
        cursor_off();
      ACTIVE_OFF();
      expose(win);

      shape(stack->x0, stack->y0, stack->esc[1], stack->esc[2]);

      ACTIVE_ON();
      dbgprintf('P', (stderr, "  position\n"));
      break;
    case P_WINDOW: /* restore the window contents */
      if (win->save)
        bit_destroy(win->save);
      win->save = bit_alloc(BIT_WIDE(win->border), BIT_HIGH(win->border),
          NULL_DATA, BIT_DEPTH(win->window));
      bit_blit(win->border, 0, 0, BIT_WIDE(stack->save), BIT_HIGH(stack->save),
          BIT_SRC, stack->save, 0, 0);
      dbgprintf('P', (stderr, "  window\n"));
      break;
    case P_FLAGS: /* restore the window flags */
      win->op = PUTOP(stack->op, win->op);
      win->style = PUTOP(stack->style, win->style);
      win->dup = stack->dup;
      win->flags = (stack->flags & W_SAVE) | (win->flags & (~W_SAVE));
      border(win, win == active ? BORDER_FAT : BORDER_THIN);
      dbgprintf('P', (stderr, "  flags\n"));
      break;
    case P_COLOR: /* restore the colors */
      win->op = PUTOP(win->op, stack->op);
      win->style = PUTOP(win->style, stack->style);
      dbgprintf('P', (stderr, "  colors\n"));
      break;
    case P_BITOP: /* restore the bitblit ops */
      win->op = PUTOP(stack->op, win->op);
      win->style = PUTOP(stack->style, win->style);
      dbgprintf('P', (stderr, "  bitblit ops\n"));
      break;
    case P_MOUSE: /* save mouse position */
      dbgprintf('P', (stderr, "  mouse\n"));
      mousex = stack->esc[3];
      mousey = stack->esc[4];
      bit_destroy(win->cursor); /* no op if static */
      win->cursor = stack->cursor;
      stack->cursor = 0;
      break;
    }
  dbgprintf('P', (stderr, "%s\n", stack->stack ? "another stack" : "no environments stacked"));
  win->stack = stack->stack;
  unlink_win(stack, 0);

  return (0);
}
/*}}}  */
