#ifndef RENDERER_H
#define RENDERER_H
#include "game.h"
#include <raylib.h>

typedef struct {
  Texture2D *frames;
  int numFrames;
  float frameSpeed;
  int currentFrame;
  float elapsedTime;
} Animation;

void InitRenderer();
void Render(Game *game);
#endif // RENDERER_H
