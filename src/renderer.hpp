#pragma once
#include "game.hpp"
#include <raylib.h>
void InitRenderer(Game *game);
void UnloadRenderer();
void Render(const Game *game);
Animation *CreateAnimation(std::vector<Texture2D> frames, float frameSpeed);
void SetCharAnimationSet(int char_id, Player *player);

constexpr int kStarFramesNum = 7;
constexpr int kBombSparkFramesNum = 2;
constexpr int kExplosionBlastFramesNum = 4;
constexpr int kCharacterSpawnFramesNum = 12;
constexpr int kCharacterIdleFramesNum = 4;
constexpr int kCharacterWalkingFramesNum = 6;
constexpr int kCharacterDeathFramesNum = 10;

extern std::vector<Texture2D> starFrames;
extern std::vector<Texture2D> bombSparkFrames;
extern std::vector<Texture2D> explosionBlastFrames;

