#ifndef UTIL_H
#define UTIL_H
#include <raylib.h>

typedef struct {
  Texture2D *frames;
  int numFrames;
  float frameSpeed;
  int currentFrame;
  float elapsedTime;
} Animation;

float Lerp(float start, float end, float t);
#endif // UTIL_H
