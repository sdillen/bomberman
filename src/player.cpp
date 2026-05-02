#include "player.hpp"
#include "game.hpp"

Player::Player(int id)
    : id_(id), position_(Grid::getSpawn(id)),
      targetPosition_(position_), speed_(5), isAlive_(true),
      state_(PlayerState::Idle), bombs_(1), blastRadius_(3) {
  switch (id) {
  case 0:
  case 3:
    facing_ = Direction::East;
    break;
  case 1:
  case 2:
    facing_ = Direction::West;
    break;
  }
}

void Player::move(Direction direction, const Grid &grid) {
  if (state_ != PlayerState::Idle) return;

  Position target = targetPosition_;
  switch (direction) {
  case Direction::North:
    target.y -= 1;
    if (!grid.isCollision(target)) targetPosition_ = target;
    break;
  case Direction::East:
    target.x += 1;
    if (!grid.isCollision(target)) {
      targetPosition_ = target;
      facing_ = Direction::East;
    }
    break;
  case Direction::South:
    target.y += 1;
    if (!grid.isCollision(target)) targetPosition_ = target;
    break;
  case Direction::West:
    target.x -= 1;
    if (!grid.isCollision(target)) {
      targetPosition_ = target;
      facing_ = Direction::West;
    }
    break;
  }
  state_ = PlayerState::Walking;
}

void Player::updatePositionProgress(float deltaTime) {
  if (state_ != PlayerState::Walking) return;

  progress_ += deltaTime * speed_;
  if (progress_ >= 1.0f) {
    progress_ = 0;
    position_ = targetPosition_;
    state_ = PlayerState::Idle;
  }
}

void Player::setState(PlayerState state) { state_ = state; }

void Player::checkAlive() {
  if (!isAlive_) state_ = PlayerState::Death;
}

void Player::kill() { isAlive_ = false; }

Animation *Player::getAnimation(PlayerState state) const {
  return animations_[static_cast<int>(state)].get();
}

void Player::setAnimation(PlayerState state, std::unique_ptr<Animation> anim) {
  animations_[static_cast<int>(state)] = std::move(anim);
}
