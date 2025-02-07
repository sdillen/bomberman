#include "renderer.h"
#include "game.h"
#include <raylib.h>
#include <stdlib.h>
#include <wchar.h>
#define TILE_SIZE 32

static Texture2D wallTexture;
static Texture2D floorTexture;
static Texture2D boxTexture;
static Texture2D bombTexture;
static Animation bombSparks;
static Animation bombExplosion;
static Animation characterIdle;
static Animation characterWalking;

const char *bombSparkFiles[] = {"assets/effects/sparks000.png",
                                "assets/effects/sparks001.png"};

const char *bombExplosionFiles[] = {"assets/effects/explosion000.png",
                                    "assets/effects/explosion002.png",
                                    "assets/effects/explosion003.png"};

const char *characterIdleFrames[] = {
    "assets/char/idle000.png", "assets/char/idle001.png",
    "assets/char/idle002.png", "assets/char/idle003.png"};

const char *characterWalkingFrames[] = {
    "assets/char/walk000.png", "assets/char/walk001.png",
    "assets/char/walk002.png", "assets/char/walk003.png",
    "assets/char/walk004.png", "assets/char/walk005.png",
};

Animation LoadAnimation(const char *fileNames[], int numFrames,
                        float frameSpeed) {
  Animation animation = {NULL, numFrames, frameSpeed, 0, 0.0f};
  animation.frames =
      (Texture2D *)malloc(sizeof(Texture2D) * animation.numFrames);
  for (int i = 0; i < animation.numFrames; i++) {
    animation.frames[i] = LoadTexture(fileNames[i]);
  }
  return animation;
}

void UpdateAnimation(Animation *animation, float deltaTime) {
  animation->elapsedTime += deltaTime;
  if (animation->elapsedTime >= animation->frameSpeed) {
    animation->elapsedTime -= animation->frameSpeed;
    animation->currentFrame =
        (animation->currentFrame + 1) % animation->numFrames;
  }
}

void InitRenderer() {
  wallTexture = LoadTexture("assets/terrain/wall.png");
  floorTexture = LoadTexture("assets/terrain/floor.png");
  boxTexture = LoadTexture("assets/items/box.png");
  bombTexture = LoadTexture("assets/items/bomb.png");
  bombSparks = LoadAnimation(bombSparkFiles, 2, 0.1f);
  bombExplosion = LoadAnimation(bombExplosionFiles, 3, 0.1f);
  characterIdle = LoadAnimation(characterIdleFrames, 4, 0.1f);
  characterWalking = LoadAnimation(characterWalkingFrames, 6, 0.1f);
}

float lerp(float start, float end, float t) {
  return start + (end - start) * t;
}

void renderMainMenu(Game *game);
void renderRunning(Game *game);
void renderPauseMenu(Game *game);

void Render(Game *game) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  switch (game->state) {
  case MAIN_MENU:
    renderMainMenu(game);
    break;
  case RUNNING:
    renderRunning(game);
    break;
  case PAUSE_MENU:
    renderRunning(game);
    renderPauseMenu(game);
    break;
  case EXIT:
    break;
  }
  EndDrawing();
}

void renderMainMenu(Game *game) {
  int fontSize = 20;
  int screenWidth = GetScreenWidth();

  DrawText(game->title,
           screenWidth / 2 - MeasureText(game->title, fontSize * 2) / 2, 50,
           fontSize * 2, BLACK);

  for (int i = 0; i < MENU_OPTIONS; i++) {
    int textWidth = MeasureText(game->mainMenu->options[i], fontSize);
    int x = screenWidth / 2 - textWidth / 2;
    int y = 150 + i * (fontSize + 15);

    if (i == game->mainMenu->selectedOption) {
      DrawText(game->mainMenu->options[i], x, y, fontSize, RED);
    } else {
      DrawText(game->mainMenu->options[i], x, y, fontSize, DARKGRAY);
    }
  }
}

void renderRunning(Game *game) {
  DrawFPS(10, 10);

  Vector2 offset = {GetScreenWidth() / 2 - (TILE_SIZE * GRID_WIDTH) / 2,
                    GetScreenHeight() / 2 - (TILE_SIZE * GRID_HEIGHT) / 2};

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell.type) {
      case CELL_EMPTY:
        DrawTextureV(floorTexture, position, WHITE);
        break;
      case CELL_SOLID_WALL:
        Rectangle rec = (Rectangle){0, 0, 32, 32};
        DrawTexturePro(
            wallTexture, rec,
            (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
            (Vector2){0, 0}, 0, WHITE);
        break;
      case CELL_DESTRUCTIBLE:
        DrawTextureV(floorTexture, position, WHITE);
        DrawTextureV(boxTexture, position, WHITE);
      default:
        break;
      }
    }
  }
  Player *player = game->player[0];
  for (int i = 0; i < MAX_BOMBS; i++) {
    if (player->bombs[i] != NULL) {
      Bomb *bomb = player->bombs[i];
      Vector2 position = {TILE_SIZE * bomb->entity.position.x + offset.x,
                          TILE_SIZE * bomb->entity.position.y + offset.y};
      if (!bomb[i].isExploded) {
        DrawTextureV(bombTexture, position, WHITE);
        UpdateAnimation(&bombSparks, game->deltaTime);
        position.y -= 8;
        DrawTextureV(bombSparks.frames[bombSparks.currentFrame], position,
                     WHITE);
      } else {
        UpdateAnimation(&bombExplosion, game->deltaTime);
        DrawTextureV(bombExplosion.frames[bombExplosion.currentFrame], position,
                     WHITE);
      }
    }
  }
  // Player
  Vector2 position = {
      lerp(player->entity.position.x, player->entity.targetPosition.x,
           player->entity.progress),
      lerp(player->entity.position.y, player->entity.targetPosition.y,
           player->entity.progress)};
  position = (Vector2){TILE_SIZE * position.x + offset.x,
                       TILE_SIZE * position.y + offset.y};
  Rectangle rec;
  if (player->entity.facing == EAST) {
    rec = (Rectangle){12, 12, 36, 36};
  } else {
    rec = (Rectangle){12, 12, -36, 36};
  }
  switch (player->state) {
  case IDLE:
    UpdateAnimation(&characterIdle, game->deltaTime);
    DrawTexturePro(characterIdle.frames[characterIdle.currentFrame], rec,
                   (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
                   (Vector2){0, 0}, 0, WHITE);
    break;
  case WALKING:
    UpdateAnimation(&characterWalking, game->deltaTime);
    DrawTexturePro(characterWalking.frames[characterWalking.currentFrame], rec,
                   (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
                   (Vector2){0, 0}, 0, WHITE);
    break;
  }
}

void renderPauseMenu(Game *game) {
  int fontSize = 20;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(BLACK, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  DrawText(game->pauseMenu->title,
           screenWidth / 2 - MeasureText(game->pauseMenu->title, fontSize) / 2,
           50, fontSize, WHITE);
}
