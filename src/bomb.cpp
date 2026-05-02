#include "bomb.hpp"

Bomb::Bomb(Position pos, int blastRadius, double currentTime, int ownerId)
    : position_(pos), endTime_(currentTime + 3), startTime_(currentTime),
      blastRadius_(blastRadius), ownerId_(ownerId) {}

bool Bomb::isTimerExpired(double currentTime) const {
  return endTime_ <= currentTime && endTime_ != 0;
}
