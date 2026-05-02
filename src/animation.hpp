#pragma once
#include <raylib.h>
#include <vector>

struct SharedFramesTag {};

class Animation {
public:
  Animation(std::vector<Texture2D> frames, float frameSpeed);
  Animation(SharedFramesTag, const std::vector<Texture2D> &sharedFrames,
            float frameSpeed);
  ~Animation();

  Animation(const Animation &) = delete;
  Animation &operator=(const Animation &) = delete;
  Animation(Animation &&other) noexcept;
  Animation &operator=(Animation &&other) noexcept;

  void update(float deltaTime);
  void draw(Vector2 position, Color color) const;
  void drawPro(Rectangle sourceRec, Rectangle destRec, Vector2 origin,
               float rotation, Color color) const;
  bool isLastFrame() const;

  const std::vector<Texture2D> &frames() const { return frames_; }
  int currentFrame() const { return currentFrame_; }

private:
  std::vector<Texture2D> frames_;
  float frameSpeed_;
  int currentFrame_{};
  float elapsedTime_{};
  bool ownsFrames_ = true;
};
