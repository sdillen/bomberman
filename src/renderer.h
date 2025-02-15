#ifndef RENDERER_H
#define RENDERER_H
#include "game.h"
#include <raylib.h>
void InitRenderer(Game *game);
void Render(Game *game);
Animation *CreateAnimation(Texture2D *frames, int numFrames, float frameSpeed);

#define STAR_FRAMES_NUM 7
extern Texture2D *starFrames;
#define BOMB_SPARK_FRAMES_NUM 2
extern Texture2D *bombSparkFrames;
#define EXPLOSION_BLAST_FRAMES_NUM 4
extern Texture2D *explosionBlastFrames;
#define CHARACTER_IDLE_FRAMES_NUM 4
extern Texture2D *characterIdleFrames;
#define CHARACTER_WALKING_FRAMES_NUM 6
extern Texture2D *characterWalkingFrames;
#endif // RENDERER_H
