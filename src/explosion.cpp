#include "explosion.hpp"

Explosion::Explosion(Position origin, Direction direction, int blastRadius,
                     float speed)
    : origin_(origin), direction_(direction), blastRadius_(blastRadius),
      speed_(speed) {
  switch (direction) {
  case Direction::North:
    targetPosition_ = {origin.x, origin.y - blastRadius};
    break;
  case Direction::East:
    targetPosition_ = {origin.x + blastRadius, origin.y};
    break;
  case Direction::South:
    targetPosition_ = {origin.x, origin.y + blastRadius};
    break;
  case Direction::West:
    targetPosition_ = {origin.x - blastRadius, origin.y};
    break;
  }
}

void Explosion::updateProgress(float deltaTime) {
  progress_ += deltaTime * speed_;
}

bool Explosion::isFinished() const { return progress_ >= 1.0f; }
