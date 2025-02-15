#include "renderer.h"
#include "game.h"
#include "log.h"
#include "util.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#define TILE_SIZE 32

#define BACKGROUND_COLOR (Color){30, 30, 30, 255}

// Raylib Logo
static Texture2D raylibLogo;

// Map
static Texture2D mapTexture;

// Items
static Texture2D boxTexture;
Texture2D *starFrames;
Animation *starAnimation;

// Bomb
static Texture2D bombTexture;
Texture2D *bombSparkFrames;
Texture2D *explosionBlastFrames;

// Character
Texture2D *characterIdleFrames;
Texture2D *characterWalkingFrames;

// File pointer
const char *starFiles[] = {
    "assets/items/star000.png", "assets/items/star001.png",
    "assets/items/star002.png", "assets/items/star003.png",
    "assets/items/star004.png", "assets/items/star005.png",
    "assets/items/star006.png"};

const char *bombSparkFiles[] = {"assets/effects/sparks000.png",
                                "assets/effects/sparks001.png"};

const char *explosionBlastFiles[] = {
    "assets/effects/blast000.png", "assets/effects/blast001.png",
    "assets/effects/blast002.png", "assets/effects/blast003.png"};

const char *characterIdleFiles[] = {
    "assets/char/idle000.png", "assets/char/idle001.png",
    "assets/char/idle002.png", "assets/char/idle003.png"};

const char *characterWalkingFiles[] = {
    "assets/char/walk000.png", "assets/char/walk001.png",
    "assets/char/walk002.png", "assets/char/walk003.png",
    "assets/char/walk004.png", "assets/char/walk005.png",
};

// Animation functions
Texture2D *loadFrames(const char *fileNames[], int numFrames);
void updateAnimation(Animation *animation, float deltaTime);
void drawAnimationV(Animation *animation, Vector2 position, Color color);
void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color);
// Entity animation functions
void loadPlayerAnimation(Player *player);
void loadBombAnimation(Bomb *bomb);
void loadExplosionAnimation(Explosion *explosion);

// Render state functions
void renderMainMenu(Game *game);
void renderRunning(Game *game);
void renderPauseMenu(Game *game);

// Render elements
Vector2 getGridOffset();
void renderStats(Game *game);
void renderMap(Game *game);
void renderPlayer(Game *game);
void renderBombs(Game *game);
void renderExplosions(Game *game);
void renderItems(Game *game);

void InitRenderer(Game *game) {
  // Textures
  LOG_DEBUG("Load static textures", NULL);
  raylibLogo = LoadTexture("assets/raylib_logo.png");
  mapTexture = LoadTexture("assets/map.png");
  boxTexture = LoadTexture("assets/items/box.png");
  bombTexture = LoadTexture("assets/items/bomb.png");
  // Frames
  LOG_DEBUG("Load static frames", NULL);
  starFrames = loadFrames(starFiles, STAR_FRAMES_NUM);
  bombSparkFrames = loadFrames(bombSparkFiles, BOMB_SPARK_FRAMES_NUM);
  explosionBlastFrames =
      loadFrames(explosionBlastFiles, EXPLOSION_BLAST_FRAMES_NUM);
  characterIdleFrames =
      loadFrames(characterIdleFiles, CHARACTER_IDLE_FRAMES_NUM);
  characterWalkingFrames =
      loadFrames(characterWalkingFiles, CHARACTER_WALKING_FRAMES_NUM);
  // Animations
  starAnimation = CreateAnimation(starFrames, STAR_FRAMES_NUM, 0.1f);
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = game->player[i];
    player->animation[IDLE] =
        CreateAnimation(characterIdleFrames, CHARACTER_IDLE_FRAMES_NUM, 0.1f);
    player->animation[WALKING] = CreateAnimation(
        characterWalkingFrames, CHARACTER_IDLE_FRAMES_NUM, 0.1f);
  }
}

void Render(Game *game) {
  LOG_DEBUG("Render", NULL);
  BeginDrawing();
  ClearBackground(BACKGROUND_COLOR);
  switch (game->state) {
  case MAIN_MENU:
    LOG_DEBUG("Render: renderMainMenu", NULL);
    renderMainMenu(game);
    break;
  case RUNNING:
    LOG_DEBUG("Render: renderRunning", NULL);
    renderRunning(game);
    break;
  case PAUSE_MENU:
    LOG_DEBUG("Render: renderRunning", NULL);
    renderRunning(game);
    LOG_DEBUG("Render: renderPauseMenu", NULL);
    renderPauseMenu(game);
    break;
  case EXIT:
    break;
  }
  EndDrawing();
}

Texture2D *loadFrames(const char *fileNames[], int numFrames) {
  Texture2D *frames = (Texture2D *)malloc(sizeof(Texture2D) * numFrames);
  if (frames == NULL) {
    LOG_ERROR("Allocation of frames failed!", NULL);
  }
  for (int i = 0; i < numFrames; i++) {
    frames[i] = LoadTexture(fileNames[i]);
    if (frames[i].id == 0) {
      LOG_ERROR("Failed to load texture for file: ", fileNames[i]);
    }
  }
  return frames;
}

Animation *CreateAnimation(Texture2D *frames, int numFrames, float frameSpeed) {
  Animation *animation = (Animation *)malloc(sizeof(Animation));
  animation->frames = frames;
  animation->numFrames = numFrames;
  animation->frameSpeed = frameSpeed;
  if (animation == NULL) {
    LOG_ERROR("Allocation of animation failed!", NULL);
  }
  return animation;
}

void updateAnimation(Animation *animation, float deltaTime) {
  LOG_DEBUG("updateAnimation", NULL);
  animation->elapsedTime += deltaTime;
  if (animation->elapsedTime >= animation->frameSpeed) {
    animation->elapsedTime -= animation->frameSpeed;
    animation->currentFrame =
        (animation->currentFrame + 1) % animation->numFrames;
  }
}

void drawAnimationV(Animation *animation, Vector2 position, Color color) {
  LOG_DEBUG("drawAnimationV: %i", animation->frames->id);
  DrawTextureV(animation->frames[animation->currentFrame], position, color);
}

void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color) {
  LOG_DEBUG("drawAnimationPro: %i", animation->frames->id);
  DrawTexturePro(animation->frames[animation->currentFrame], sourceRec, destRec,
                 origin, rotation, color);
}

void renderMainMenu(Game *game) {
  int fontSize = TILE_SIZE;
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  int maxTileWidth = screenWidth / TILE_SIZE;
  int maxTileHeight = screenHeight / TILE_SIZE;

  DrawText(game->title,
           screenWidth / 2 - MeasureText(game->title, fontSize * 2) / 2,
           TILE_SIZE * 2, fontSize * 2, WHITE);

  DrawRectangleLines(maxTileWidth / 2 * TILE_SIZE - TILE_SIZE * 6, TILE_SIZE,
                     TILE_SIZE * 12, TILE_SIZE * 4, WHITE);

  for (int i = 0; i < MENU_OPTIONS; i++) {
    int textWidth = MeasureText(game->mainMenu->options[i], fontSize);
    int x = screenWidth / 2 - textWidth / 2;
    int y = (TILE_SIZE) * 6 + (i * TILE_SIZE * 2);

    if (i == game->mainMenu->selectedOption) {
      DrawText(game->mainMenu->options[i], x, y, fontSize, RED);
      DrawLine(x, y + TILE_SIZE, x + textWidth, y + TILE_SIZE, RED);
    } else {
      DrawText(game->mainMenu->options[i], x, y, fontSize, WHITE);
    }
  }

  DrawText("Erstellt von Stefan Dillenburg", TILE_SIZE,
           screenHeight - TILE_SIZE, TILE_SIZE / 2, WHITE);
  DrawText("Powered by", (maxTileWidth - 7) * TILE_SIZE,
           screenHeight - TILE_SIZE, TILE_SIZE / 4, WHITE);
  DrawTexture(raylibLogo, (maxTileWidth - 5) * TILE_SIZE,
              (maxTileHeight - 4) * TILE_SIZE, WHITE);
}

void renderRunning(Game *game) {
  LOG_DEBUG("renderRunning: renderStats", NULL);
  renderStats(game);
  LOG_DEBUG("renderRunning: renderMap", NULL);
  renderMap(game);
  LOG_DEBUG("renderRunning: renderItems", NULL);
  renderItems(game);
  LOG_DEBUG("renderRunning: renderBombs", NULL);
  renderBombs(game);
  LOG_DEBUG("renderRunning: renderPlayer", NULL);
  renderPlayer(game);
  LOG_DEBUG("renderRunning: renderExplosions", NULL);
  renderExplosions(game);
}

void renderPauseMenu(Game *game) {
  int fontSize = TILE_SIZE;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(BACKGROUND_COLOR, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  DrawText(game->pauseMenu->title,
           screenWidth / 2 - MeasureText(game->pauseMenu->title, fontSize) / 2,
           TILE_SIZE, fontSize, WHITE);
}

Vector2 getGridOffset() {
  return (Vector2){GetScreenWidth() / 2 - (TILE_SIZE * GRID_WIDTH) / 2,
                   GetScreenHeight() / 2 - (TILE_SIZE * GRID_HEIGHT) / 2};
}

void renderMap(Game *game) {
  Vector2 offset = getGridOffset();
  DrawTextureV(mapTexture, offset, WHITE);
}

void renderStats(Game *game) {
  int fontSize = TILE_SIZE;

  // FPS
  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           GetScreenWidth() - MeasureText(fpsText, fontSize / 2) - TILE_SIZE,
           TILE_SIZE, fontSize / 2, WHITE);

  DrawRectangleLines(TILE_SIZE, TILE_SIZE, TILE_SIZE * 7, TILE_SIZE * 5, WHITE);

  // Player Look
  Texture2D characterTexture = game->player[0]->animation[IDLE]->frames[0];
  Rectangle source = (Rectangle){12, 12, 36, 36};
  DrawTexturePro(
      characterTexture, source,
      (Rectangle){TILE_SIZE * 2 - 8, TILE_SIZE * 2 - 8, TILE_SIZE, TILE_SIZE},
      (Vector2){0, 0}, 0, WHITE);

  // Speed
  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f", game->player[0]->speed);
  DrawText(speedText, TILE_SIZE * 2, TILE_SIZE * 3, fontSize / 2, WHITE);

  // Bombs
  DrawText("Bomben:", TILE_SIZE * 2, TILE_SIZE * 4, fontSize / 2, WHITE);
  for (int i = 0; i < game->player[0]->bombs; i++) {
    if (game->player[0]->bombList[i] == NULL) {
      DrawTextureV(bombTexture,
                   (Vector2){MeasureText("Bomben:", fontSize / 2) +
                                 TILE_SIZE * 2 + (TILE_SIZE * i) + 8,
                             TILE_SIZE * 4 - 8},
                   WHITE);
    }
  }

  // Blast-Radius
  char blastRadiusText[100];
  sprintf(blastRadiusText, "Explosionsradius: %i",
          game->player[0]->blastRadius);
  DrawText(blastRadiusText, TILE_SIZE * 2, TILE_SIZE * 5, fontSize / 2, WHITE);

  // Controls
  char *explainControlsText =
      "[W],[S],[A],[D] Bewegen\n[SPACE] Bombe\n[P] Pause\n";
  DrawText(explainControlsText, TILE_SIZE, GetScreenHeight() - TILE_SIZE * 2,
           fontSize / 4, WHITE);
}

void renderPlayer(Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = game->player[i];
    Vector2 position = {
        Lerp(player->entity.position.x, player->entity.targetPosition.x,
             player->entity.progress),
        Lerp(player->entity.position.y, player->entity.targetPosition.y,
             player->entity.progress)};
    position = (Vector2){TILE_SIZE * position.x + offset.x,
                         TILE_SIZE * position.y + offset.y - 8};
    Rectangle rec;
    if (player->entity.facing == EAST) {
      rec = (Rectangle){12, 12, 36, 36};
    } else {
      rec = (Rectangle){12, 12, -36, 36};
    }
    LOG_DEBUG("player->state: %i:%i", i, player->state);
    switch (player->state) {
    case IDLE:
      drawAnimationPro(
          player->animation[IDLE], rec,
          (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
          (Vector2){0, 0}, 0, WHITE);
      updateAnimation(player->animation[IDLE], game->deltaTime);
      break;
    case WALKING:
      drawAnimationPro(
          player->animation[WALKING], rec,
          (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
          (Vector2){0, 0}, 0, WHITE);
      updateAnimation(player->animation[WALKING], game->deltaTime);
      break;
    default:
      break;
    }
  }
}

void renderBombs(Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = game->player[i];
    for (int j = 0; j < player->bombs; j++) {
      if (player->bombList[j] != NULL) {
        Bomb *bomb = player->bombList[j];
        if (bomb->endTime != 0) {
          Vector2 position = {TILE_SIZE * bomb->entity.position.x + offset.x,
                              TILE_SIZE * bomb->entity.position.y + offset.y};
          DrawTextureV(bombTexture, position, WHITE);
          position.y -= 8;
          drawAnimationV(bomb->animation, position, WHITE);
          updateAnimation(bomb->animation, game->deltaTime);
        }
      }
    }
  }
}

void renderExplosions(Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = game->player[i];
    for (int j = 0; j < player->bombs; j++) {
      Bomb *bomb = player->bombList[j];
      if (bomb != NULL && bomb->endTime == 0) {
        for (int k = 0; k < _DIRECTION_NUM; k++) {
          if (bomb->explosion[k] != NULL) {
            Explosion *explosion = bomb->explosion[k];
            float x, y;
            x = Lerp(explosion->entity.position.x,
                     explosion->entity.targetPosition.x,
                     explosion->entity.progress);
            y = Lerp(explosion->entity.position.y,
                     explosion->entity.targetPosition.y,
                     explosion->entity.progress);
            Vector2 v = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
            Rectangle rec;
            int rotation = 0;
            Vector2 origin = {0, 0};
            switch (k) {
            case NORTH:
              rec = (Rectangle){0, 0, 32, 32};
              rotation = 270;
              origin = (Vector2){32, 0};
              break;
            case EAST:
              rec = (Rectangle){0, 0, 32, 32};
              break;
            case SOUTH:
              rec = (Rectangle){0, 0, 32, 32};
              rotation = 90;
              origin = (Vector2){0, 32};
              break;
            case WEST:
              rec = (Rectangle){0, 0, -32, 32};
              break;
            }
            drawAnimationPro(explosion->animation, rec,
                             (Rectangle){v.x, v.y, TILE_SIZE, TILE_SIZE},
                             origin, rotation, WHITE);
            updateAnimation(explosion->animation, game->deltaTime);
          }
        }
      }
    }
  }
}

void renderItems(Game *game) {
  Vector2 offset = getGridOffset();
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell.type) {
      case CELL_DESTRUCTIBLE:
        DrawTextureV(boxTexture, position, WHITE);
        break;
      case CELL_POWERUP:
        drawAnimationV(starAnimation, position, WHITE);
        updateAnimation(starAnimation, game->deltaTime);
      default:
        break;
      }
    }
  }
}
