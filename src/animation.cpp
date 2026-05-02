#include "animation.hpp"

Animation::Animation(std::vector<Texture2D> frames, float frameSpeed)
    : frames_(std::move(frames)), frameSpeed_(frameSpeed) {}

Animation::Animation(SharedFramesTag,
                     const std::vector<Texture2D> &sharedFrames,
                     float frameSpeed)
    : frames_(sharedFrames), frameSpeed_(frameSpeed), ownsFrames_(false) {}

Animation::Animation(Animation &&other) noexcept
    : frames_(std::move(other.frames_)), frameSpeed_(other.frameSpeed_),
      currentFrame_(other.currentFrame_), elapsedTime_(other.elapsedTime_),
      ownsFrames_(other.ownsFrames_) {
  other.ownsFrames_ = false;
}

Animation &Animation::operator=(Animation &&other) noexcept {
  if (this != &other) {
    if (ownsFrames_) {
      for (auto &frame : frames_) {
        UnloadTexture(frame);
      }
    }
    frames_ = std::move(other.frames_);
    frameSpeed_ = other.frameSpeed_;
    currentFrame_ = other.currentFrame_;
    elapsedTime_ = other.elapsedTime_;
    ownsFrames_ = other.ownsFrames_;
    other.ownsFrames_ = false;
  }
  return *this;
}

Animation::~Animation() {
  if (ownsFrames_) {
    for (auto &frame : frames_) {
      UnloadTexture(frame);
    }
  }
}

void Animation::update(float deltaTime) {
  if (frames_.empty()) return;
  elapsedTime_ += deltaTime;
  if (elapsedTime_ >= frameSpeed_) {
    elapsedTime_ -= frameSpeed_;
    currentFrame_ = (currentFrame_ + 1) % frames_.size();
  }
}

void Animation::draw(Vector2 position, Color color) const {
  if (frames_.empty()) return;
  DrawTextureV(frames_[currentFrame_], position, color);
}

void Animation::drawPro(Rectangle sourceRec, Rectangle destRec,
                        Vector2 origin, float rotation, Color color) const {
  if (frames_.empty()) return;
  DrawTexturePro(frames_[currentFrame_], sourceRec, destRec, origin, rotation,
                 color);
}

bool Animation::isLastFrame() const {
  if (frames_.empty()) return false;
  return static_cast<int>(frames_.size()) - 1 == currentFrame_;
}
