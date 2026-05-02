#pragma once
#include "animation.hpp"
#include "grid.hpp"
#include <memory>
#include <array>

enum class Direction {
  North,
  East,
  South,
  West,
};

enum class PlayerState {
  Spawn,
  Idle,
  Walking,
  Death,
};

constexpr int kDirectionCount = 4;
constexpr int kPlayerStateCount = 4;

class Player {
public:
  Player(int id);

  void move(Direction direction, const Grid &grid);
  void updatePositionProgress(float deltaTime);
  void setState(PlayerState state);
  void checkAlive();
  void kill();

  int getId() const { return id_; }
  Position getPosition() const { return position_; }
  Position getTargetPosition() const { return targetPosition_; }
  float getProgress() const { return progress_; }
  Direction getFacing() const { return facing_; }
  float getSpeed() const { return speed_; }
  bool isAlive() const { return isAlive_; }
  PlayerState getState() const { return state_; }
  int getBombs() const { return bombs_; }
  int getBlastRadius() const { return blastRadius_; }

  void setPosition(Position pos) { position_ = pos; }
  void setTargetPosition(Position pos) { targetPosition_ = pos; }
  void setProgress(float p) { progress_ = p; }
  void setFacing(Direction d) { facing_ = d; }
  void setSpeed(float s) { speed_ = s; }
  void setBombs(int b) { bombs_ = b; }
  void setBlastRadius(int r) { blastRadius_ = r; }

  void addBombCount() { bombs_++; }
  void addSpeed() { speed_++; }
  void addBlastRadius() { blastRadius_++; }

  Animation *getAnimation(PlayerState state) const;
  void setAnimation(PlayerState state, std::unique_ptr<Animation> anim);

private:
  int id_;
  Position position_;
  Position targetPosition_;
  float progress_{};
  Direction facing_;
  float speed_;
  bool isAlive_;
  PlayerState state_;
  int bombs_;
  int blastRadius_;
  std::array<std::unique_ptr<Animation>, kPlayerStateCount> animations_;
};
