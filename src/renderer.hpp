#pragma once
#include "game.hpp"
#include <raylib.h>
#include <memory>
#include <vector>

constexpr int kStarFramesNum = 7;
constexpr int kBombSparkFramesNum = 2;
constexpr int kExplosionBlastFramesNum = 4;
constexpr int kCharacterSpawnFramesNum = 12;
constexpr int kCharacterIdleFramesNum = 4;
constexpr int kCharacterWalkingFramesNum = 6;
constexpr int kCharacterDeathFramesNum = 10;

class Renderer {
public:
  static void init(Game *game);
  static void unload();
  static void render(const Game *game);
  static void setCharAnimationSet(int char_id, Player *player);

  static const std::vector<Texture2D> &getBombSparkFrames();
  static const std::vector<Texture2D> &getExplosionBlastFrames();

private:
  static std::vector<Texture2D> bombSparkFrames_;
  static std::vector<Texture2D> explosionBlastFrames_;
  static std::vector<Texture2D> starFrames_;
  static std::unique_ptr<Animation> starAnimation_;

  static Texture2D raylibLogo_;
  static Texture2D arrowTexture_;
  static Texture2D mapTexture_;
  static Texture2D crateTexture_;
  static Texture2D bombTexture_;

  static std::vector<Texture2D> loadFrames(const char *fileNames[],
                                            int numFrames);

  static void renderMainMenu(const Game *game);
  static void renderCharSelectMenu(const Game *game);
  static void renderRunningCountdown(const Game *game);
  static void renderRunning(const Game *game);
  static void renderPauseMenu(const Game *game);

  static void renderStats(const Game *game);
  static void renderMap(const Game *game);
  static void renderPlayer(const Game *game);
  static void renderBombs(const Game *game);
  static void renderExplosions(const Game *game);
  static void renderItems(const Game *game);

  static Vector2 getGridOffset();
  static void drawCenteredText(const char *text, int pos_y, int font_size,
                                Color color);
  static void renderPlayerAnimation(Animation *animation, Rectangle source,
                                     Rectangle dest, float deltaTime);
  static int getCharAnimationFiles(const char *file_names[], int num_frames,
                                    PlayerState state, int char_id);
  static void loadCharacterAnimation(Player *player, PlayerState state,
                                      int char_id);
};
