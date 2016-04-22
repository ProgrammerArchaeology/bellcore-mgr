/*{{{}}}*/
/*{{{  Notes*/
/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
*/

/* down load text -- called from put_window.c */
/*}}}  */
/*{{{  #includes*/
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mgr/bitblit.h>
#include <mgr/font.h>

#include "defs.h"
#include "menu.h"
#include "event.h"

#include "proto.h"
#include "do_event.h"
#include "do_menu.h"
#include "font_subs.h"
#include "get_font.h"
#include "get_menus.h"
#include "win_subs.h"
#include "write_ok.h"
/*}}}  */

/*{{{  get_map -- find bitmap associated with window id*/
static BITMAP *
get_map(
    int id, /* pid of process controlling window */
    int sub /* window number of this window */
    )
{
  WINDOW *win;
  BITMAP *map;

  for (win = active; win != NULL; win = win->next)
    if (win->pid == id && win->num == sub) {
      map = bit_alloc(BIT_WIDE(win->window), BIT_HIGH(win->window), NULL, BIT_DEPTH(win->window));
      if (map && win->flags & W_ACTIVE)
        bit_blit(map, 0, 0, BIT_WIDE(map), BIT_HIGH(map), BIT_SRC, win->window, 0, 0);
      else if (map)
        bit_blit(map, 0, 0, BIT_WIDE(map), BIT_HIGH(map), BIT_SRC, win->save, win->borderwid, win->borderwid);
      return (map);
    }
  return (NULL);
}
/*}}}  */

/*{{{  down_load*/
void down_load(WINDOW *win, BITMAP *window, BITMAP *text)
{
  WINDOW *win2;
  int cnt;
  int id;

  cnt = win->esc_cnt;
  switch (win->code) {
  /*{{{  T_INVALID -- invalid text mode*/
  case T_INVALID:
    dbgprintf('w', (stderr, "mgr: invalid download code\n"));
    break;
  /*}}}  */
  /*{{{  T_MENU    -- down load menu*/
  case T_MENU: {
    struct font *f;
    int fn = win->esc[1];

    if (*win->snarf && cnt > 1)
      f = Get_font(fn);
    else
      f = win->font;

    if (*win->snarf) {
      win->menus
          [*win->esc]
          = do_menu(win->snarf, f, win->style);
      if (active == win) {
        if (win->menu[0] == *win->esc && button_state == BUTTON_2)
          go_menu(0);
        else if (win->menu[1] == *win->esc && button_state == BUTTON_1)
          go_menu(1);
      }
    } else
      win->menus
          [*win->esc]
          = NULL;
  } break;
  /*}}}  */
  /*{{{  T_EVENT   -- down load an event*/
  case T_EVENT:
    cnt = win->esc[0];
    if (!CHK_EVENT(cnt)) {
      break;
    }
    if (win->events[GET_EVENT(cnt)]) {
      free(win->events[GET_EVENT(cnt)]);
      win->events
          [GET_EVENT(cnt)]
          = NULL;
    }
    if (*win->snarf) {
      win->events
          [GET_EVENT(cnt)]
          = win->snarf;
      win->snarf = NULL;
      EVENT_SET_MASK(win, cnt);
      dbgprintf('e', (stderr, "%s: setting event %d (%zu)[%s]\r\n",
                         win->tty, GET_EVENT(cnt), strlen(win->snarf), win->snarf));
      /* if button is down, then do the associated event */

      if (win == active && ((cnt == EVENT_B1_DOWN && button_state == BUTTON_1) || (cnt == EVENT_B2_DOWN && button_state == BUTTON_2)))
        do_event(button_state, win, E_MAIN);
    } else {
      EVENT_CLEAR_MASK(win, cnt);
      dbgprintf('e', (stderr, "%s: clearing event %d\r\n", win->tty, GET_EVENT(cnt)));
    }
    break;
  /*}}}  */
  /*{{{  T_STRING  -- draw text into offscreen bitmap*/
  case T_STRING: {
    int x = cnt > 1 ? Scalex(win->esc[1]) : 0;
    int y = cnt > 2 ? Scaley(win->esc[2]) : 0;

    if (y < FSIZE(high))
      y = FSIZE(high);
    if (x < 0)
      x = 0;
    dbgprintf('y', (stderr, "%s: drawing [%s] to %d\r\n",
                       win->tty, win->snarf, *win->esc));
    if (*win->esc > 0 && win->bitmaps[*win->esc - 1] == NULL) {
      win->bitmaps
          [*win->esc - 1]
          = bit_alloc(x + strlen(win->snarf) * FSIZE(wide), y, NULL, 1); /* text is always 1 bit deep */
      dbgprintf('y', (stderr, "%s: STRING creating %d (%zux%d)\n",
                         win->tty, *win->esc, x + strlen(win->snarf) * FSIZE(wide), y));
    }
    if (*win->esc > 0)
      put_str(win->bitmaps[*win->esc - 1], x, y, win->font, win->op, win->snarf);
    else
      put_str(window, x, y, win->font, win->op, win->snarf);
  } break;
  /*}}}  */
  /*{{{  T_YANK    -- fill yank buffer*/
  case T_YANK:
    if (snarf)
      free(snarf);
    snarf = win->snarf;
    dbgprintf('y', (stderr, "%s: yanking [%s]\r\n", win->tty, snarf));
    id_message = win->pid;
    win->snarf = NULL;
    for (win2 = active; win2 != NULL; win2 = win2->next)
      do_event(EVENT_SNARFED, win2, E_MAIN);
    break;

  /*}}}  */
  /*{{{  T_SEND    -- send a message*/
  case T_SEND:
    id = *win->esc;
    if (message) {
      free(message);
      message = NULL;
    }
    message = win->snarf;
    id_message = win->pid;
    win->snarf = NULL;
    dbgprintf('e', (stderr, "%s: sending [%s]\r\n", win->tty, win->snarf));
    dbgprintf('c', (stderr, "sending %d->%d: %s\r\n", win->pid, cnt == 0 ? 0 : id, message));
    for (win2 = active; win2 != NULL; win2 = win2->next)
      if (cnt == 0 || win2->pid == id) {
        do_event(EVENT_ACCEPT, win2, E_MAIN);
        if (cnt)
          break;
      }
    break;
  /*}}}  */
  /*{{{  T_GMAP    -- load bitmap from a file*/
  case T_GMAP: {
    BITMAP *b;
    FILE *fp = NULL;
    char filename[MAX_PATH];
    char buff[20];
    char c = *win->snarf;
    int bmi = win->esc[0] - 1;

    /* make relative to icon directory */

    if (c == '/' || (c == '.' && win->snarf[1] == '/'))
      strcpy(filename, win->snarf);
    else
      sprintf(filename, "%s/%s", icon_dir, win->snarf);

    if (win->flags & W_DUPKEY)
      sprintf(buff, "%c ", win->dup);
    else
      *buff = '\0';

    if (bmi >= 0 && bmi < MAXBITMAPS
        && read_ok(filename)
        && (fp = fopen(filename, "r")) != NULL
        && (b = bitmapread(fp))) {
      if (win->bitmaps[bmi])
        bit_destroy(win->bitmaps[bmi]);
      win->bitmaps[bmi] = b;
      sprintf(buff + strlen(buff), "%d %d %d\n", BIT_WIDE(b), BIT_HIGH(b), BIT_DEPTH(b));
    } else {
      strcat(buff, "\n");
    }
    write(win->to_fd, buff, strlen(buff));

    if (fp != NULL)
      fclose(fp);
  } break;
  /*}}}  */
  /*{{{  T_SMAP    -- save a bitmap on a file*/
  case T_SMAP: {
    FILE *fp;
    BITMAP *b = 0;
    int exists; /* file already exists */
    int free_b = 0;
    int num = *win->esc;

    switch (cnt) {
    case 1: /* off screen bitmap */
      if (num > 0)
        b = win->bitmaps[num - 1];
      else
        b = screen;
      break;
    case 0: /* my window */
      free_b++;
      b = bit_alloc(BIT_WIDE(window), BIT_HIGH(window), NULL, BIT_DEPTH(window));
      if (b)
        bit_blit(b, 0, 0, BIT_WIDE(b), BIT_HIGH(b), BIT_SRC, window, 0, 0);
      break;
    case 2: /* other guy's window */
      free_b++;
      b = get_map(num, win->esc[1]);
      break;
    }

    dbgprintf('y', (stderr, "saving...\n"));
    if (b && win->snarf && ((exists = access(win->snarf, 0)),
                               write_ok(win->snarf))
        && (fp = fopen(win->snarf, "w")) != NULL) {
      dbgprintf('y', (stderr, "saving bitmap %d x %d on %s (%d)\n",
                         BIT_WIDE(b), BIT_HIGH(b), win->snarf, fileno(fp)));
      if (exists < 0) /* file just created */
        fchown(fileno(fp), getuid(), getgid());
      bitmapwrite(fp, b);
      fclose(fp);
      dbgprintf('y', (stderr, "saved on %s\n", win->snarf));
    }

    if (b && free_b)
      bit_destroy(b);
  } break;
  /*}}}  */
  /*{{{  T_GIMME   -- send to process*/
  case T_GIMME:
    if (win->snarf && *win->snarf)
      write_event(win, win->snarf, E_LIST_UP);
    dbgprintf('y', (stderr, "%s: sending [%s]\r\n", win->tty, win->snarf));
    break;
  /*}}}  */
  /*{{{  T_GRUNCH  -- graphics scrunch mode (experimental)*/
  case T_GRUNCH:
    if (win->snarf)
      grunch(win, window);
    dbgprintf('y', (stderr, "%s: grunching [%d]\r\n", win->tty, win->esc[cnt]));
    break;
  /*}}}  */
  /*{{{  T_FONT    -- change a font name*/
  case T_FONT:
    if (win->esc[0] <= MAXFONT && win->esc[0] > 0) {
      if (fontlist[win->esc[0] - 1])
        free(fontlist[win->esc[0] - 1]);
      fontlist[win->esc[0] - 1] = win->snarf;
      win->snarf = NULL;
    }
    break;
  /*}}}  */
  /*{{{  T_BITMAP  -- down load a bitmap*/
  case T_BITMAP:
    win_map(win, window);
    break;
    /*}}}  */
  }
  if (win->snarf) {
    free(win->snarf);
    win->snarf = 0;
  }
}
/*}}}  */
