/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

/* destroy a window */
/*}}}  */
/*{{{  #includes*/
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>
#include <mgr/window.h>

#include "clip.h"
#include "defs.h"
#include "event.h"

#include "proto.h"
#include "border.h"
#include "colormap.h"
#include "do_event.h"
#include "erase_win.h"
#include "font_subs.h"
#include "get_menus.h"
#include "icon_server.h"
#include "put_window.h"
#include "subs.h"
#include "update.h"
#include "utmp.h"
/*}}}  */
/*{{{  #defines*/
#define ALL 1

#ifdef WEXITSTATUS /* POSIX */
#define wait_nohang(statusp) waitpid(-1, statusp, WNOHANG)
#else /* BSD */
#define wait_nohang(statusp) wait3(statusp, WNOHANG, NULL)
#endif
/*}}}  */

/*{{{  detach -- unlink an alternate window from list*/
static void
detach(WINDOW *win2)
{
  WINDOW *win = win2;

  if (!(win->main))
    return;
  for (win = win2->main; win->alt != win2; win = win->alt)
    ;
  win->alt = win2->alt;
}
/*}}}  */
/*{{{  set_dead -- notify alternate windows of impending death*/
static void
set_dead(WINDOW *win)
{
  for (win = win->alt; win != NULL; win = win->alt) {
    dbgprintf('d', (stderr, "%s: telling %d\r\n", win->tty, win->num));
    win->main = NULL;
  }
}
/*}}}  */

/*{{{  unlink_win -- free all space associated with a window*/
void unlink_win(
    WINDOW *win, /* window to unlink */
    int how      /* if how, unlink window stack as well */
    )
{
  int i;

  dbgprintf('u', (stderr, "Unlinking %s %s\n", win->tty, how ? "ALL" : ""));

  if (how && win->stack)
    unlink_win(win->stack, how);
  if (win->window)
    bit_destroy(win->window);
  for (i = 0; i < MAXBITMAPS; i++)
    if (win->bitmaps[i])
      bit_destroy(win->bitmaps[i]);
  bit_destroy(win->cursor); /* usually noop because static */
  if (win->border)
    bit_destroy(win->border);
  if (win->save)
    bit_destroy(win->save);
  if (win->snarf)
    free(win->snarf);
  if (win->bitmap)
    free(win->bitmap);
  zap_cliplist(win);

  for (i = 0; i < MAXEVENTS; i++)
    if (win->events[i])
      free(win->events[i]);

  for (i = 0; i < MAXMENU; i++)
    if (win->menus[i])
      menu_destroy(win->menus[i]);

  free(win);
  win = NULL;
}
/*}}}  */
/*{{{  destroy -- destroy a window*/
int destroy(WINDOW *win)
{
  int status;

  if (win == NULL)
    return (-1);

  MOUSE_OFF(screen, mousex, mousey);
  cursor_off();

  if (win != active) {
    ACTIVE_OFF();
    expose(win);
  }

  active = win->next;

  /* deallocate window slot */

  if (active)
    active->prev = win->prev;

  /* remove window from screen */

  erase_win(win->border);

  if (win->main == win) { /* kill process associated with the window */
    dbgprintf('d', (stderr, "%s: destroy main %s\r\n", win->tty, win->alt ? "ALT" : ""));
    if (win->pid > 1)
      killpg(win->pid, SIGHUP);

    if (geteuid() < 1) {
      chmod(win->tty, 0666);
      chown(win->tty, 0, 0);
    }

    close(win->to_fd);
    FD_CLR(win->to_fd, &mask);
    FD_CLR(win->to_fd, &to_poll);
#ifdef WHO
    rm_utmp(win->tty);
#endif
    free_colors(win, 0, 255); /* release claims on the colormap */

    /* tell alternate windows main died */

    set_dead(win);

    /* wait for shell to die */

    dbgprintf('d', (stderr, "waiting for ..."));
    fflush(stderr);
    if (win->pid > 1 && !(win->flags & W_DIED)) {
      int wpid;

      wpid = wait_nohang(&status);
      if (wpid == 0) { /* start it so it can die */
        kill(win->pid, SIGCONT);
        wpid = wait_nohang(&status);
        if (wpid == 0)
          fprintf(stderr, "MGR: Wait for %d failed\n", win->pid);
      }
      dbgprintf('d', (stderr, "wait_nohang returns %d\r\n", wpid));
    }
    next_window--;
  }

  else if (win->main && !(win->main->flags & W_DIED)) { /* main still alive */
    dbgprintf('d', (stderr, "%s: destroy alt %d\r\n", win->tty, win->num));
    do_event(EVENT_DESTROY, win, E_MAIN);
    if (win->from_fd) { /* re-attach output to main window */
      win->main
          ->from_fd
          = win->main->to_fd;
      win->main
          ->max
          = win->max - win->current; /* ??? */
      dbgprintf('d', (stderr, "%s: copy %d chars at %d\r\n",
                         win->tty, win->main->max, win->current));
      memcpy(win->main->buff, win->buff + win->current + 1, win->main->max);
      win->main
          ->current
          = 0;
      set_size(win);
      dbgprintf('d', (stderr, "%s: reattaching main %d chars\r\n", win->tty, win->max));
    }
    detach(win);
  } else if (win->main) { /* tell main alts know they are dead */
    win->main
        ->alt
        = NULL;
    dbgprintf('d', (stderr, "%s: destroy alt, (tell main)\r\n", win->tty));
  } else {
    dbgprintf('d', (stderr, "%s: destroy alt, (dead main)\r\n", win->tty));
  }

  /* fix up display if any windows left */

  SETMOUSEICON(DEFAULT_MOUSE_CURSOR); /* because active win chg */

  if (active) {
    repair(win);
    un_covered();
    clip_bad(win); /* invalidate clip lists */
    ACTIVE_ON();
    cursor_on();
  }

  /* free space associated with dead window */

  unlink_win(win, ALL);

  dbgprintf('d', (stderr, "Active: %s-%d\r\n",
                     active ? active->tty : "NONE", active ? active->num : 0));

  MOUSE_ON(screen, mousex, mousey);

  return (0);
}
/*}}}  */
/*{{{  destroy_window -- mark active window for destruction*/
void destroy_window(void)
{
  active->flags |= W_DIED;
}
/*}}}  */
