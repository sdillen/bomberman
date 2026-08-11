#pragma once
#include "animation.hpp"
#include "grid.hpp"
#include "player.hpp"
#include <memory>

class Explosion {
public:
  Explosion(Position origin, Direction direction, int blastRadius,
            float speed);

  void updateProgress(float deltaTime);
  bool isFinished() const;

  Position getOrigin() const { return origin_; }
  Position getTargetPosition() const { return targetPosition_; }
  Direction getDirection() const { return direction_; }
  float getProgress() const { return progress_; }
  Animation *getAnimation() const { return animation_.get(); }
  void setAnimation(std::unique_ptr<Animation> anim) {
    animation_ = std::move(anim);
  }

private:
  Position origin_;
  Position targetPosition_;
  Direction direction_;
  int blastRadius_;
  float progress_{};
  float speed_;
  std::unique_ptr<Animation> animation_;
};
