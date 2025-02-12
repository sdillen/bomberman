#include "renderer.h"
#include "game.h"
#include <raylib.h>
#include <stdlib.h>
#include <wchar.h>
#define TILE_SIZE 32

#define BACKGROUND_COLOR (Color){30, 30, 30, 255}
// Map
static Texture2D mapTexture;
// Items
static Texture2D boxTexture;
static Animation star;
// Bomb
static Texture2D bombTexture;
static Animation bombSparks;
static Animation explosionBlast;
// Character
static Animation characterIdle;
static Animation characterWalking;

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
  mapTexture = LoadTexture("assets/map.png");
  boxTexture = LoadTexture("assets/items/box.png");
  bombTexture = LoadTexture("assets/items/bomb.png");
  star = LoadAnimation(starFiles, 7, 0.1f);
  bombSparks = LoadAnimation(bombSparkFiles, 2, 0.1f);
  explosionBlast = LoadAnimation(explosionBlastFiles, 4, 0.1f);
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
  ClearBackground(BACKGROUND_COLOR);
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
           fontSize * 2, WHITE);

  for (int i = 0; i < MENU_OPTIONS; i++) {
    int textWidth = MeasureText(game->mainMenu->options[i], fontSize);
    int x = screenWidth / 2 - textWidth / 2;
    int y = 150 + i * (fontSize + 15);

    if (i == game->mainMenu->selectedOption) {
      DrawText(game->mainMenu->options[i], x, y, fontSize, RED);
    } else {
      DrawText(game->mainMenu->options[i], x, y, fontSize, WHITE);
    }
  }
}

void renderRunning(Game *game) {
  int fontSize = 20;
  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           GetScreenWidth() - MeasureText(fpsText, fontSize) - TILE_SIZE,
           TILE_SIZE, fontSize, WHITE);

  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f", game->player[0]->speed);
  DrawText(speedText, TILE_SIZE, TILE_SIZE * 1, fontSize, WHITE);

  DrawText("Bomben:", TILE_SIZE, TILE_SIZE * 2, fontSize, WHITE);
  for (int i = 0; i < game->player[0]->bombs; i++) {
    if (game->player[0]->bombList[i] == NULL) {
      DrawTextureV(bombTexture,
                   (Vector2){MeasureText("Bomben:", fontSize) + TILE_SIZE +
                                 (TILE_SIZE * i) + 8,
                             TILE_SIZE * 2 - 8},
                   WHITE);
    }
  }

  char blastRadiusText[100];
  sprintf(blastRadiusText, "Explosionsradius: %i",
          game->player[0]->blastRadius);
  DrawText(blastRadiusText, TILE_SIZE, TILE_SIZE * 3, fontSize, WHITE);

  char *explainControlsText =
      "[W],[S],[A],[D] Bewegen\n[SPACE] Bombe\n[P] Pause\n";
  DrawText(explainControlsText,
           MeasureText(explainControlsText, fontSize / 1.5f),
           GetScreenHeight() - TILE_SIZE * 2, fontSize / 1.5f, WHITE);

  Vector2 offset = {GetScreenWidth() / 2 - (TILE_SIZE * GRID_WIDTH) / 2,
                    GetScreenHeight() / 2 - (TILE_SIZE * GRID_HEIGHT) / 2};

  DrawTextureV(mapTexture, offset, WHITE);
  UpdateAnimation(&star, game->deltaTime);

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell.type) {
      case CELL_DESTRUCTIBLE:
        DrawTextureV(boxTexture, position, WHITE);
        break;
      case CELL_POWERUP:
        DrawTextureV(star.frames[star.currentFrame], position, WHITE);
      default:
        break;
      }
    }
  }
  // Bombs & Explosion
  Player *player = game->player[0];
  for (int i = 0; i < player->bombs; i++) {
    if (player->bombList[i] != NULL) {
      Bomb *bomb = player->bombList[i];
      if (bomb->endTime != 0) {
        Vector2 position = {TILE_SIZE * bomb->entity.position.x + offset.x,
                            TILE_SIZE * bomb->entity.position.y + offset.y};
        DrawTextureV(bombTexture, position, WHITE);
        UpdateAnimation(&bombSparks, game->deltaTime);
        position.y -= 8;
        DrawTextureV(bombSparks.frames[bombSparks.currentFrame], position,
                     WHITE);
      } else {
        UpdateAnimation(&explosionBlast, game->deltaTime);
        for (int i = 0; i < 4; i++) {
          if (bomb->explosion[i] != NULL) {
            Explosion *explosion = bomb->explosion[i];
            float x, y;
            x = lerp(explosion->entity.position.x,
                     explosion->entity.targetPosition.x,
                     explosion->entity.progress);
            y = lerp(explosion->entity.position.y,
                     explosion->entity.targetPosition.y,
                     explosion->entity.progress);
            Vector2 v = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
            Rectangle rec;
            int rotation = 0;
            Vector2 origin = {0, 0};
            switch (i) {
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
            DrawTexturePro(explosionBlast.frames[explosionBlast.currentFrame],
                           rec, (Rectangle){v.x, v.y, TILE_SIZE, TILE_SIZE},
                           origin, rotation, WHITE);
          }
        }
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
                       TILE_SIZE * position.y + offset.y - 8};
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
  int fontSize = 40;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(BLACK, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  DrawText(game->pauseMenu->title,
           screenWidth / 2 - MeasureText(game->pauseMenu->title, fontSize) / 2,
           50, fontSize, WHITE);
}
