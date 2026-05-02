#pragma once
#include <raylib.h>
#include <vector>

struct Animation {
  std::vector<Texture2D> frames;
  float frameSpeed;
  int currentFrame;
  float elapsedTime;
};

float Lerp(float start, float end, float t);

