/*                        Copyright (c) 1987 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */

#include <stdio.h>
#include <SDL.h>
#include "screen.h"

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} DeviceInfo;

DATA *
bit_initscreen(char *name, int *width, int *height, unsigned char *depth, void **device)
{
  printf("bit_initscreen(%s)\n", name);

  DeviceInfo *deviceInfo = malloc(sizeof(DeviceInfo));

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return NULL;
  }

  if (SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE, &(deviceInfo->window), &(deviceInfo->renderer))) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
    return NULL;
  }

  *width = 640;
  *height = 480;
  *depth = 32;
  *device = deviceInfo;
  return NULL;
}

void display_close(BITMAP *bitmap)
{
  printf("display_close(%p)\n", bitmap);

  DeviceInfo * deviceInfo = (DeviceInfo *)bitmap->deviceinfo;

  if (deviceInfo == NULL)
    return;

  SDL_DestroyRenderer(deviceInfo->renderer);
  SDL_DestroyWindow(deviceInfo->window);

  free(deviceInfo);
  bitmap->deviceinfo = NULL;

  SDL_Quit();
}
