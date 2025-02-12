#include "renderer.h"
#include "game.h"
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
static Animation star;

// Bomb
static Texture2D bombTexture;
static Animation bombSparks;
static Animation explosionBlast;

// Character
static Animation characterIdle;
static Animation characterWalking;

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

const char *characterIdleFrames[] = {
    "assets/char/idle000.png", "assets/char/idle001.png",
    "assets/char/idle002.png", "assets/char/idle003.png"};

const char *characterWalkingFrames[] = {
    "assets/char/walk000.png", "assets/char/walk001.png",
    "assets/char/walk002.png", "assets/char/walk003.png",
    "assets/char/walk004.png", "assets/char/walk005.png",
};

// Animation functions
Animation loadAnimation(const char *fileNames[], int numFrames,
                        float frameSpeed);
void updateAnimation(Animation *animation, float deltaTime);
void drawAnimationV(Animation *animation, Vector2 position, Color color);
void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color);
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

void InitRenderer() {
  // Textures
  raylibLogo = LoadTexture("assets/raylib_logo.png");
  mapTexture = LoadTexture("assets/map.png");
  boxTexture = LoadTexture("assets/items/box.png");
  bombTexture = LoadTexture("assets/items/bomb.png");
  // Animations
  star = loadAnimation(starFiles, 7, 0.1f);
  bombSparks = loadAnimation(bombSparkFiles, 2, 0.1f);
  explosionBlast = loadAnimation(explosionBlastFiles, 4, 0.1f);
  characterIdle = loadAnimation(characterIdleFrames, 4, 0.1f);
  characterWalking = loadAnimation(characterWalkingFrames, 6, 0.1f);
}

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

Animation loadAnimation(const char *fileNames[], int numFrames,
                        float frameSpeed) {
  Animation animation = {NULL, numFrames, frameSpeed, 0, 0.0f};
  animation.frames =
      (Texture2D *)malloc(sizeof(Texture2D) * animation.numFrames);
  for (int i = 0; i < animation.numFrames; i++) {
    animation.frames[i] = LoadTexture(fileNames[i]);
  }
  return animation;
}

void updateAnimation(Animation *animation, float deltaTime) {
  animation->elapsedTime += deltaTime;
  if (animation->elapsedTime >= animation->frameSpeed) {
    animation->elapsedTime -= animation->frameSpeed;
    animation->currentFrame =
        (animation->currentFrame + 1) % animation->numFrames;
  }
}

void drawAnimationV(Animation *animation, Vector2 position, Color color) {
  DrawTextureV(animation->frames[animation->currentFrame], position, color);
}

void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color) {
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
  renderStats(game);
  renderMap(game);
  renderItems(game);
  renderBombs(game);
  renderPlayer(game);
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
  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           GetScreenWidth() - MeasureText(fpsText, fontSize / 2) - TILE_SIZE,
           TILE_SIZE, fontSize / 2, WHITE);

  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f", game->player[0]->speed);
  DrawText(speedText, TILE_SIZE, TILE_SIZE * 1, fontSize / 2, WHITE);

  DrawText("Bomben:", TILE_SIZE, TILE_SIZE * 2, fontSize / 2, WHITE);
  for (int i = 0; i < game->player[0]->bombs; i++) {
    if (game->player[0]->bombList[i] == NULL) {
      DrawTextureV(bombTexture,
                   (Vector2){MeasureText("Bomben:", fontSize / 2) + TILE_SIZE +
                                 (TILE_SIZE * i) + 8,
                             TILE_SIZE * 2 - 8},
                   WHITE);
    }
  }

  char blastRadiusText[100];
  sprintf(blastRadiusText, "Explosionsradius: %i",
          game->player[0]->blastRadius);
  DrawText(blastRadiusText, TILE_SIZE, TILE_SIZE * 3, fontSize / 2, WHITE);

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
    switch (player->state) {
    case IDLE:
      updateAnimation(&characterIdle, game->deltaTime);
      drawAnimationPro(
          &characterIdle, rec,
          (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
          (Vector2){0, 0}, 0, WHITE);
      break;
    case WALKING:
      updateAnimation(&characterWalking, game->deltaTime);
      drawAnimationPro(
          &characterWalking, rec,
          (Rectangle){position.x, position.y, TILE_SIZE, TILE_SIZE},
          (Vector2){0, 0}, 0, WHITE);
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
          updateAnimation(&bombSparks, game->deltaTime);
          position.y -= 8;
          drawAnimationV(&bombSparks, position, WHITE);
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
        updateAnimation(&explosionBlast, game->deltaTime);
        for (int k = 0; k < 4; k++) {
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
            drawAnimationPro(&explosionBlast, rec,
                             (Rectangle){v.x, v.y, TILE_SIZE, TILE_SIZE},
                             origin, rotation, WHITE);
          }
        }
      }
    }
  }
}

void renderItems(Game *game) {
  Vector2 offset = getGridOffset();
  updateAnimation(&star, game->deltaTime);
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell.type) {
      case CELL_DESTRUCTIBLE:
        DrawTextureV(boxTexture, position, WHITE);
        break;
      case CELL_POWERUP:
        drawAnimationV(&star, position, WHITE);
      default:
        break;
      }
    }
  }
}
