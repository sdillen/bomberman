#include "animation.hpp"

namespace {
void unloadFrames(const std::vector<Texture2D> *frames) {
  for (auto &frame : *frames) UnloadTexture(frame);
  delete frames;
}
} // namespace

Animation::Animation(std::vector<Texture2D> frames, float frameSpeed)
    : frames_(new std::vector<Texture2D>(std::move(frames)), unloadFrames),
      frameSpeed_(frameSpeed) {}

Animation::Animation(std::shared_ptr<const std::vector<Texture2D>> frames,
                     float frameSpeed)
    : frames_(std::move(frames)), frameSpeed_(frameSpeed) {}

void Animation::update(float deltaTime) {
  if (frames().empty()) return;
  elapsedTime_ += deltaTime;
  if (elapsedTime_ >= frameSpeed_) {
    elapsedTime_ -= frameSpeed_;
    currentFrame_ = (currentFrame_ + 1) % frames().size();
  }
}

void Animation::draw(Vector2 position, Color color) const {
  if (frames().empty()) return;
  DrawTextureV(frames().at(currentFrame_), position, color);
}

void Animation::drawPro(Rectangle sourceRec, Rectangle destRec,
                        Vector2 origin, float rotation, Color color) const {
  if (frames().empty()) return;
  DrawTexturePro(frames().at(currentFrame_), sourceRec, destRec, origin,
                 rotation, color);
}

bool Animation::isLastFrame() const {
  if (frames().empty()) return false;
  return static_cast<int>(frames().size()) - 1 == currentFrame_;
}
