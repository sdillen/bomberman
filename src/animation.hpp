#pragma once
#include <raylib.h>
#include <memory>
#include <vector>

class Animation {
public:
  Animation(std::vector<Texture2D> frames, float frameSpeed);
  Animation(std::shared_ptr<const std::vector<Texture2D>> frames,
            float frameSpeed);

  void update(float deltaTime);
  void draw(Vector2 position, Color color) const;
  void drawPro(Rectangle sourceRec, Rectangle destRec, Vector2 origin,
               float rotation, Color color) const;
  bool isLastFrame() const;

  const std::vector<Texture2D> &frames() const {
    static const std::vector<Texture2D> kEmpty;
    return frames_ ? *frames_ : kEmpty;
  }
  int currentFrame() const { return currentFrame_; }

private:
  std::shared_ptr<const std::vector<Texture2D>> frames_;
  float frameSpeed_;
  int currentFrame_{};
  float elapsedTime_{};
};
