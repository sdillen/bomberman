#include "util.hpp"

float Lerp(float start, float end, float t) {
  return start + (end - start) * t;
}
