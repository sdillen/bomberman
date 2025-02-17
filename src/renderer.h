#ifndef RENDERER_H
#define RENDERER_H
#include "game.h"
#include <raylib.h>
void InitRenderer(Game *game);
void Render(Game *game);
Animation *CreateAnimation(Texture2D *frames, int numFrames, float frameSpeed);
void SetCharAnimationSet(int char_id, Player *player);

#define STAR_FRAMES_NUM 7
extern Texture2D *starFrames;
#define BOMB_SPARK_FRAMES_NUM 2
extern Texture2D *bombSparkFrames;
#define EXPLOSION_BLAST_FRAMES_NUM 4
extern Texture2D *explosionBlastFrames;
#define CHARACTER_SPAWN_FRAMES_NUM 12
extern Texture2D *characterSpawnFrames;
#define CHARACTER_IDLE_FRAMES_NUM 4
extern Texture2D *characterIdleFrames;
#define CHARACTER_WALKING_FRAMES_NUM 6
extern Texture2D *characterWalkingFrames;
#define CHARACTER_DEATH_FRAMES_NUM 10
extern Texture2D *characterDeathFrames;
#endif // RENDERER_H
