/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
*/

/*

Very ugly stuff to use machine dependent things.

*/
/*}}}  */
/*{{{  #includes*/
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include <mgr/bitblit.h>

#include "proto.h"

#include "defs.h"
/*}}}  */

/*{{{  variables*/
#ifdef BELL
#ifdef sun
static int ring = 0;
static int bell_fd = -1;
#endif
#endif
/*}}}  */

/*{{{  initbell -- initialize bell*/
void initbell(void)
{
#ifdef BELL
#ifdef sun
  int i;
  if (!debug && (i = open("/dev/bell", O_WRONLY)) >= 0)
    bell_fd = i;
  else
    bell_fd = -1;
#endif
#endif
}
/*}}}  */
/*{{{  bell_on -- turn on the bell*/
#ifdef BELL
#ifdef sun
/*{{{  set_timer*/
static void
set_timer(
    int time /* time in 100'th of seconds */
    )
{
  struct itimerval new, old;

  new.it_interval.tv_sec = 0L;
  new.it_interval.tv_usec = 0L;
  new.it_value.tv_sec = time / 100;
  new.it_value.tv_usec = (time % 100) * 10000;

  setitimer(ITIMER_REAL, &new, &old);
}
/*}}}  */
/*{{{  bell_off*/
int bell_off(
    int n /* signal #, ignored */
    )
{
  write(bell_fd, "\003", 1); /* turns the bell off */
  set_timer(0);              /* insure timer is off */
  ring = 0;
}
/*}}}  */
#endif
#endif

void bell_on(void)
{
#ifdef BELL
#ifdef sun
  if (bell_fd >= 0) {
    if (ring == 0)
      write(bell_fd, "\002", 1);
    signal(SIGALRM, bell_off);
    set_timer(15); /* set alarm for 100'th seconds */
    ring++;
    return;
  }
#endif
#ifdef linux
  /* This does not work on Suns, because you are logged in on /dev/console,
  so console redirection brings that prompt back as output to the window ...
  On Linux you can't login with /dev/console as terminal, so this is safe. */
  write(fileno(stderr), "\007", 1);
#endif
#endif
}
/*}}}  */

/*{{{  kbd_reset -- reset the keyboard*/
void kbd_reset(void)
{
#ifdef BELL
#ifdef sun
  if (bell_fd >= 0)
    write(bell_fd, "\001", 1); /* this resets the kbd and turns bell off */
  set_timer(0);                /* insure timer is off */
  ring = 0;
#endif
#endif
}
/*}}}  */

/*{{{  set_kdb -- set/reset direct mode*/
/* When setting direct mode, returns file descriptor of keyboard */
int set_kbd(
    int how /* 1=direct, 0=no direct */
    )
{
  return (0);
}
/*}}}  */
/*{{{  initkbd -- initialize the keyboard, especially when it is a separate device*/
void initkbd(void)
{
}
/*}}}  */
