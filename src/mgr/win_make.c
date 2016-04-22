/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* make an alternate window */
/*}}}  */
/*{{{  #includes*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "proto.h"
#include "do_button.h"
#include "do_event.h"
#include "border.h"
#include "font_subs.h"
#include "icon_server.h"
#include "new_window.h"
#include "put_window.h"
#include "subs.h"
#include "update.h"
/*}}}  */

/*{{{  win_make -- manipulate an alternate (client) window - called by put_window()*/
void win_make(WINDOW *win, int indx)
/* window issuing make-call */
/* current index into char string (yuk!) */
{
  int *p = W(esc); /* array of ESC digits */
  char buff[20];
  WINDOW *win2 = win;

  switch (W(esc_cnt)) {
  case 1: /*  destroy the window */
    dbgprintf('N', (stderr, "%s: destroying %d\n", W(tty), p[0]));
    if (p[0] <= 0 || W(main)->alt == NULL) {
      break;
    }
    for (win = W(main)->alt; win != NULL; win = W(alt)) {
      if (W(num) == p[0])
        break;
    }
    if (win != NULL)
      W(flags) |= W_DIED;

    break;

  case 0: /* goto a new window */
    if (W(num) == p[0] || W(main)->alt == NULL) {
      break;
    }
    for (win = W(main); win != NULL; win = W(alt)) {
      if (W(num) == p[0])
        break;
    }

    /* move contents of shell buffer to new window */

    if (win != NULL) {
      W(from_fd) = W(to_fd);
      win2->from_fd = 0;
      W(max) = win2->max - win2->current - indx - 1;
      (void)memcpy(W(buff), win2->buff + win2->current + indx + 1, W(max));
      W(current) = 0;
      dbgprintf('N', (stderr, "%s: xfer %d\r\n", W(tty), W(max)));
      set_size(win);
    }
    break;

  case 3: /* make a new window */
    p[4] = -1;
  /* no break */
  case 4: /* new window + specify window number */
    dbgprintf('N', (stderr, "%s: making alternate window\n", W(tty)));
    if (check_window(p[0], p[1], p[2], p[3], -1) == 0) {
      if (active->flags & W_DUPKEY)
        sprintf(buff, "%c \n", active->dup);
      else
        sprintf(buff, "\n");
      write(active->to_fd, buff, strlen(buff));
      break;
    }

    if (win != active)
      cursor_off();
    ACTIVE_OFF();
    if ((active = insert_win(NULL)) == NULL || !setup_window(active, font, p[0], p[1], p[2], p[3])) {
      fprintf(stderr, "Out of memory for window creation -- bye!\n");
      quit();
    }

    /* next_window++;  (this needs more thought) */

    /* make the window */

    set_covered(active);
    border(active, BORDER_THIN);
    CLEAR(active->window, BIT_CLR);
    SETMOUSEICON(DEFAULT_MOUSE_CURSOR); /* because active win chg */
    ACTIVE_ON();
    cursor_on();

    dbgprintf('N', (stderr, "%s: window created\n", W(tty)));
    /* fix pointer chain */

    active->to_fd = W(to_fd);
    active->main = W(main);
    active->pid = W(pid);
    active->setid = W(setid);
    strcpy(active->tty, active->main->tty);
    active->from_fd = 0;
    active->alt = W(main)->alt;
    active->main
        ->alt
        = active;
    if (p[4] > 0)
      active->num = p[4];
    else if (active->alt)
      active->num = active->alt->num + 1;
    else
      active->num = 1;

    dbgprintf('N', (stderr, "%s: created num %d\r\n", active->tty, active->num));
    if (W(flags) & W_DUPKEY)
      sprintf(buff, "%c %d\n", W(dup), active->num);
    else
      sprintf(buff, "%d\n", active->num);
    write(active->to_fd, buff, strlen(buff));
    clip_bad(active); /* invalidate clip lists */
    break;
  case 5: /* nothing */
    break;
  }
}
/*}}}  */
