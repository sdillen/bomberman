#pragma once
#include "animation.hpp"
#include "grid.hpp"
#include "explosion.hpp"
#include <memory>
#include <array>

class Bomb {
public:
  Bomb(Position pos, int blastRadius, double currentTime);

  Position getPosition() const { return position_; }
  double getEndTime() const { return endTime_; }
  double getStartTime() const { return startTime_; }
  bool isExploded() const { return isExploded_; }
  bool isTimerExpired(double currentTime) const;
  int getBlastRadius() const { return blastRadius_; }

  Animation *getAnimation() const { return animation_.get(); }
  void setAnimation(std::unique_ptr<Animation> anim) {
    animation_ = std::move(anim);
  }

  std::array<std::unique_ptr<Explosion>, kDirectionCount> &getExplosions() {
    return explosions_;
  }

  void setEndTime(double t) { endTime_ = t; }
  void markExploded() { isExploded_ = true; }
  void clearExplosion(int index) { explosions_[index].reset(); }

private:
  Position position_;
  double endTime_;
  double startTime_;
  int blastRadius_;
  bool isExploded_{};
  std::array<std::unique_ptr<Explosion>, kDirectionCount> explosions_;
  std::unique_ptr<Animation> animation_;
};
