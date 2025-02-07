#include "renderer.h"
#include "game.h"
#include <raylib.h>
#include <stdlib.h>
#define TILE_SIZE 32

static Animation characterIdle;
static Animation characterWalking;

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
  characterIdle = LoadAnimation(characterIdleFrames, 4, 0.1f);
  characterWalking = LoadAnimation(characterWalkingFrames, 6, 0.05f);
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
  Vector2 offset = {GetScreenWidth() % (TILE_SIZE * GRID_WIDTH) / 2,
                    GetScreenHeight() % (TILE_SIZE * GRID_HEIGHT) / 2};
  Vector2 size = {TILE_SIZE, TILE_SIZE};

  DrawFPS(10, 10);

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell.type) {
      case CELL_EMPTY:
        DrawRectangleV(position, size, WHITE);
        break;
      case CELL_SOLID_WALL:
        DrawRectangleV(position, size, BLACK);
        break;
      default:
        break;
      }
    }
  }
  // Player
  Player *player = game->player[0];
  Vector2 position = {
      lerp(player->entity.position.x, player->entity.targetPosition.x,
           player->entity.progress),
      lerp(player->entity.position.y, player->entity.targetPosition.y,
           player->entity.progress)};
  position = (Vector2){TILE_SIZE * position.x + offset.x,
                       TILE_SIZE * position.y + offset.y};
  Rectangle rec;
  if (player->entity.facing == EAST) {
    rec = (Rectangle){0, 0, 64, 64};
  } else {
    rec = (Rectangle){0, 0, -64, 64};
  }
  switch (player->state) {
  case IDLE:
    UpdateAnimation(&characterIdle, game->deltaTime);
    DrawTexturePro(characterIdle.frames[characterIdle.currentFrame], rec,
                   (Rectangle){position.x, position.y, 32, 32}, (Vector2){0, 0},
                   0, WHITE);
    break;
  case WALKING:
    UpdateAnimation(&characterWalking, game->deltaTime);
    DrawTexturePro(characterWalking.frames[characterWalking.currentFrame], rec,
                   (Rectangle){position.x, position.y, 32, 32}, (Vector2){0, 0},
                   0, WHITE);
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
