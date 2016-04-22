/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* do a button event */
/*}}}  */
/*{{{  #includes*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "defs.h"
#include "event.h"

#include "proto.h"
#include "Write.h"
#include "border.h"
#include "do_button.h"
#include "font_subs.h"
#include "get_rect.h"
#include "get_text.h"
#include "intersect.h"
#include "move_box.h"
#include "write_ok.h"
/*}}}  */
/*{{{  #defines*/
#define SUB_SIZE 256  /* max temp str size */
#define START_SIZE 16 /* default starting size of sweep object */
/*}}}  */

/*{{{  event_args -- extract numeric argument from sweep events*/
static char *
event_args(
    char *str,  /*  beginning of args */
    int *count, /* # of args */
    int *args   /* where to put args */
    )
{
  char c, *pntr = str; /* 1st char of args */

  while (((c = *++pntr) >= '0' && c <= '9') || c == ',' || c == '-')
    ;
  *count = sscanf(str + 1, "%d,%d,%d,%d", args, args + 1, args + 2, args + 3);
  return (pntr);
}
/*}}}  */
/*{{{  get_id -- compute a unique window id*/
static char *
get_id(WINDOW *win)
{
  int sub = win->num;  /* subwindow number */
  int main = win->pid; /* main window id */
  static char buff[6];

  sprintf(buff, "%d.%d", main, sub);
  return (buff);
}
/*}}}  */
/*{{{  sub_event -- substitute %x into str, returns true if an area was swept.*/
static int
sub_event(
    WINDOW *win,
    char *str,
    int c,
    int swept, /* if swept, don't do sweeps */
    int count, /* # of sweep args */
    int *args  /* the arg list */
    )
{
  int sweep = 0;
  static int x, y;
  char *get_id();
  int code; /* for text sweeping */

#ifdef DEBUG
  if (debug) {
    int i;
    dbgprintf('e', (stderr, "%s) event (%c) args:", win->tty, c));
    for (i = 0; i < count; i++)
      dbgprintf('e', (stderr, " %d", args[i]));
    dbgprintf('e', (stderr, "\r\n"));
  }
#endif

  /* setup initial sweep conditions */

  if (swept == 0) { /* no sweep - set up initial conditions */
    if (count >= 2) {
      x = args[0];
      y = args[1];
    } else if (c == E_SWTEXT || c == E_SWTEXTT) {
      x = 1;
      y = 0;
    } else {
      x = START_SIZE;
      y = START_SIZE;
    }
    dbgprintf('e', (stderr, "initial sweep (x,y) = (%d,%d)\r\n", x, y));
    count = 0;
  }

  switch (c) {
  case E_CPOS: /* return mouse position (rows/cols) */
    sprintf(str, "%d %d", (mousex - (win->x0 + win->borderwid + win->text.x)) / FSIZE(wide),
        (mousey - (win->y0 + win->borderwid + win->text.y)) / FSIZE(high));
    break;
  case E_POS: /* return mouse position */
    if (win->flags & W_ABSCOORDS)
      sprintf(str, "%d %d", mousex - win->x0, mousey - win->y0);
    else
      sprintf(str, "%ld %ld", (mousex - win->x0) * GMAX / BIT_WIDE(win->window),
          (mousey - win->y0) * GMAX / BIT_HIGH(win->window));
    break;
  case E_SWLINE: /* sweep out line */
    sweep++;
    if (!swept)
      get_rect(screen, mouse, mousex, mousey, &x, &y, 1);
    sprintf(str, "%d %d %d %d", mousex - win->x0, mousey - win->y0,
        mousex + x - win->x0, mousey + y - win->y0);
    break;
  case E_SWRECT: /* sweep out rectangle */
    sweep++;
    if (!swept)
      get_rect(screen, mouse, mousex, mousey, &x, &y, 0);
    sprintf(str, "%d %d %d %d", mousex - win->x0, mousey - win->y0,
        mousex + x - win->x0, mousey + y - win->y0);
    break;
  case E_SWRECTA: /* sweep out rectangle */
    sweep++;
    if (!swept)
      get_rect(screen, mouse, mousex, mousey, &x, &y, 0);
    sprintf(str, "%d %d %d %d", mousex, mousey,
        mousex + x, mousey + y);
    break;
  case E_SWBOX: /* sweep out box */
    sweep++;
    if (!swept)
      move_box(screen, mouse, &mousex, &mousey, x, y, 1);
    sprintf(str, "%d %d", mousex - win->x0, mousey - win->y0);
    break;
  case E_SWBOXA: /* sweep out box */
    sweep++;
    if (!swept)
      move_box(screen, mouse, &mousex, &mousey, x, y, 1);
    sprintf(str, "%d %d", mousex, mousey);
    break;
  case E_SWTEXTT: /* sweep out text */
  case E_SWTEXT:  /* sweep out text */
    sweep++;
    code = 0;
    if (!swept)
      code = get_text(screen, mouse, mousex, mousey, &x, &y, win, c);
    if (code)
      sprintf(str, "%d %d %d %d",
          (mousex - (win->x0 + win->borderwid + win->text.x)) / FSIZE(wide),
          (mousey - (win->y0 + win->borderwid + win->text.y)) / FSIZE(high),
          x, y);
    else
      *str = '\0';
    break;
  case E_NOTIFY: /* get other windows notify text */
    for (win = active; win != NULL; win = win->next) {
      if (mousein(mousex, mousey, win, 1))
        break;
    }
    if (win && IS_EVENT(win, EVENT_NOTIFY))
      sprintf(str, "%.*s", SUB_SIZE - 1, win->events[GET_EVENT(EVENT_NOTIFY)]);
    else
      *str = '\0';
    break;
  case E_WHO: /* send other windows id */
    for (win = active; win != NULL; win = win->next) {
      if (mousein(mousex, mousey, win, 1))
        break;
    }
    if (win)
      sprintf(str, "%.*s", SUB_SIZE - 1, get_id(win));
    else
      *str = '\0';
    break;
  case E_WINSIZE: /* send other windows size */
    sprintf(str, "%d %d %d %d",
        win->x0, win->y0, BIT_WIDE(win->border), BIT_HIGH(win->border));
    break;
  case E_WHOSIZE: /* send other windows size */
    for (win = active; win != NULL; win = win->next) {
      if (mousein(mousex, mousey, win, 1))
        break;
    }
    if (win)
      sprintf(str, "%d %d %d %d",
          win->x0, win->y0, BIT_WIDE(win->border), BIT_HIGH(win->border));
    else
      *str = '\0';
    break;
  case E_FROM: /* see who message is from */
    sprintf(str, "%d", id_message);
    break;
  case E_MESS:
    if (message)
      sprintf(str, "%.*s", SUB_SIZE - 1, message);
    else
      *str = '\0';
    break;
  case E_MSGSIZE:
    sprintf(str, "%d", message ? (int)strlen(message) : 0);
    break;
  case E_SNARFSIZE: /* size of snarf buffer */
    sprintf(str, "%d", snarf ? (int)strlen(snarf) : 0);
    break;
  case E_SNARFBUF: /* contents of snarf buffer */
    if (snarf)
      sprintf(str, "%.*s", SUB_SIZE - 1, snarf);
    else
      *str = '\0';
    break;
  case E_TIMESTAMP: /* 100ths seconds since MGR startup */
    sprintf(str, "%d", timestamp());
    break;
  case E_ESC: /* the escape char */
    strcpy(str, "%");
    break;
  }
  return (sweep);
}
/*}}}  */

/*{{{  write_event -- write the event to a process*/
void write_event(
    WINDOW *win, /* window to get info about */
    char *str,   /* event string */
    char *list   /* list of valid event chars */
    )
{
  char data[SUB_SIZE];
  int args[4]; /* arguments to sweep event */
  int count;   /* # of args */
  char *start;
  char *end;
  int swept = 0; /* already did a sweep */

  for (start = str; *start && (end = strchr(start, E_ESC)); start = end + 1) {
    dbgprintf('e', (stderr, "  sending %d [%s]\r\n", (int)strlen(str), str));
    if (end > start)
      Write(win->to_fd, start, end - start);
    end = event_args(end, &count, args);
    if (strchr(list, *end)) {
      swept += sub_event(win, data, *end, swept, count, args);
      Write(win->to_fd, data, strlen(data));
    }
  }
  if (*start)
    Write(win->to_fd, start, strlen(start));
  if (swept)
    /* If we swept something, the button was down and is now up.  Notify
	 do_button(). */
    do_button(0);
}
/*}}}  */
/*{{{  do_event -- do a button event*/
void do_event(
    int event,   /* event number */
    WINDOW *win, /* window event applies to */
    int flag     /* type of window */
    )
{
  char *buff;

  if (!win)
    return;

  dbgprintf('e', (stderr, "%s: event %d (%s) %s\r\n",
                     win->tty, GET_EVENT(event),
                     IS_EVENT(win, event) ? "ON" : "OFF",
                     flag == E_MAIN ? "MAIN" : "STACK"));

  /* look for stacked events */

  if (IS_EVENT(win, EVENT_STFLAG)) {
    do_event(event, win->stack, E_STACK);
  }

  if (IS_EVENT(win, event) && (flag == E_MAIN || IS_EVENT(win, EVENT_STACK))) {

    dbgprintf('e', (stderr, "\tSENT\r\n"));

    /* do the event */

    switch (event) {
    case EVENT_B1_DOWN:
    case EVENT_B2_DOWN:
      if (IS_EVENT(win, event) && (buff = win->events[GET_EVENT(event)]))
        write_event(win, buff, E_LIST_BUTTON);

      /* notify clicked window */

      for (win = active; win != NULL; win = win->next)
        if (mousein(mousex, mousey, win, 1))
          break;
      if (win && IS_EVENT(win, EVENT_TELLME)
          && (buff = win->events[GET_EVENT(EVENT_TELLME)])) {
        if (message) {
          free(message);
          message = NULL;
        }
        id_message = active->pid;
        write_event(win, buff, E_LIST_ACCEPT);
      }
      break;
    case EVENT_PASTE:
      if (IS_EVENT(win, event) && (buff = win->events[GET_EVENT(event)]))
        write_event(win, buff, E_LIST_PASTE);
      break;
    case EVENT_SNARFED:
      if (IS_EVENT(win, event) && (buff = win->events[GET_EVENT(event)]))
        write_event(win, buff, E_LIST_SNARF);
      break;
    case EVENT_BSYS_DOWN: /* No events for System Button, down or up. */
    case EVENT_BSYS_UP:
      break;
    case EVENT_B1_UP:
    case EVENT_B2_UP:

    case EVENT_SHAPE:
    case EVENT_MOVE:
    case EVENT_DESTROY:
    case EVENT_REDRAW:
    case EVENT_COVERED:
    case EVENT_UNCOVERED:
    case EVENT_DEACTIVATED:
    case EVENT_ACTIVATED:
      if (IS_EVENT(win, event) && (buff = win->events[GET_EVENT(event)]))
        write_event(win, buff, E_LIST_UP);
      break;
    case EVENT_ACCEPT:
      buff = win->events[GET_EVENT(event)];
      if (buff && message && mode_ok(win->tty, MSG_MODEMASK)) {
        dbgprintf('e', (stderr, "  accept: %d:  [%s]\r\n",
                           (int)strlen(buff), buff));
        dbgprintf('c', (stderr, "  sent %d->%d: %s\r\n",
                           id_message, win->pid, message));
        write_event(win, buff, E_LIST_ACCEPT);
      }
#ifdef DEBUG
      else {
        dbgprintf('c', (stderr,
                           "%d: can't send [%s] to %s\r\n",
                           id_message,
                           message ? message : "??",
                           win->tty));
        dbgprintf('e', (stderr,
                           "  reject accept: %s %s %s\r\n",
                           mode_ok(win->tty, MSG_MODEMASK)
                               ? "OK"
                               : "BAD_MODE",
                           message ? message : "NO MESSAGE",
                           buff ? buff : "NO EVENT"));
      }
#endif
      break;
    default:
      dbgprintf('e', (stderr, "  oops! unknown event\r\n"));
    } /* end switch */
  }
  return;
}
/*}}}  */
