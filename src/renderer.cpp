#include "renderer.hpp"
#include "game.hpp"
#include "log.hpp"
#include "util.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
constexpr int kTileSize = 32;

constexpr Color kBackgroundColor = {30, 30, 30, 255};

// Raylib Logo
static Texture2D raylibLogo;

// UI
static Texture2D arrowTexture;

// Map
static Texture2D mapTexture;

// Items
static Texture2D crateTexture;
std::vector<Texture2D> starFrames;
Animation *starAnimation;

// Bomb
static Texture2D bombTexture;
std::vector<Texture2D> bombSparkFrames;
std::vector<Texture2D> explosionBlastFrames;

// File pointer
const char *starFiles[] = {
    "assets/items/star000.png", "assets/items/star001.png",
    "assets/items/star002.png", "assets/items/star003.png",
    "assets/items/star004.png", "assets/items/star005.png",
    "assets/items/star006.png",
};

const char *bombSparkFiles[] = {
    "assets/effects/sparks000.png",
    "assets/effects/sparks001.png",
};

const char *explosionBlastFiles[] = {
    "assets/effects/blast000.png",
    "assets/effects/blast001.png",
    "assets/effects/blast002.png",
    "assets/effects/blast003.png",
};

// Animation functions
std::vector<Texture2D> loadFrames(const char *fileNames[], int numFrames);
void updateAnimation(Animation *animation, float deltaTime);
void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color);

// Draw functions
void drawCenteredText(const char *text, int pos_y, int font_size, Color color);

// Entity animation functions
void loadPlayerAnimation(Player *player);
void loadBombAnimation(Bomb *bomb);
void loadExplosionAnimation(Explosion *explosion);

// Render state functions
void renderMainMenu(const Game *game);
void renderCharSelectMenu(const Game *game);
void renderRunningCountdown(const Game *game);
void renderRunning(const Game *game);
void renderPauseMenu(const Game *game);

// Render elements
Vector2 getGridOffset();
void renderStats(const Game *game);
void renderMap(const Game *game);
void renderPlayer(const Game *game);
void renderBombs(const Game *game);
void renderExplosions(const Game *game);
void renderItems(const Game *game);

void InitRenderer(Game *game) {
  // Textures
  LOG_DEBUG("Load static textures", nullptr);
  raylibLogo = LoadTexture("assets/raylib_logo.png");
  arrowTexture = LoadTexture("assets/ui/arrow.png");
  mapTexture = LoadTexture("assets/map.png");
  crateTexture = LoadTexture("assets/items/crate.png");
  bombTexture = LoadTexture("assets/items/bomb.png");
  for (int i = 0; i < kCharacters; i++) {
    char path[256];
    sprintf(path, "assets/characters/%02d/idle/sprite_000.png", i);
    game->charSelectMenu->charTexture[i] = LoadTexture(path);
  }
  // Frames
  LOG_DEBUG("Load static frames", nullptr);
  starFrames = loadFrames(starFiles, kStarFramesNum);
  bombSparkFrames = loadFrames(bombSparkFiles, kBombSparkFramesNum);
  explosionBlastFrames =
      loadFrames(explosionBlastFiles, kExplosionBlastFramesNum);
  // Animations
  starAnimation = CreateAnimation(starFrames, 0.1f);
}

void UnloadRenderer() {
  UnloadTexture(raylibLogo);
  UnloadTexture(arrowTexture);
  UnloadTexture(mapTexture);
  UnloadTexture(crateTexture);
  UnloadTexture(bombTexture);
  delete starAnimation;
}

void Render(const Game *game) {
  LOG_DEBUG("Render", nullptr);
  BeginDrawing();
  ClearBackground(kBackgroundColor);
  switch (game->state) {
  case GameStateType::MainMenu:
    LOG_DEBUG("Render: renderMainMenu", nullptr);
    renderMainMenu(game);
    break;
  case GameStateType::RunningCountdown:
    LOG_DEBUG("Render: renderRunning", nullptr);
    renderRunning(game);
    LOG_DEBUG("Render: renderRunningCountdown", nullptr);
    renderRunningCountdown(game);
    break;
  case GameStateType::CharSelectMenu:
    LOG_DEBUG("Reder: renderCharSelectMenu", nullptr);
    renderCharSelectMenu(game);
    break;
  case GameStateType::Running:
    LOG_DEBUG("Render: renderRunning", nullptr);
    renderRunning(game);
    break;
  case GameStateType::PauseMenu:
    LOG_DEBUG("Render: renderRunning", nullptr);
    renderRunning(game);
    LOG_DEBUG("Render: renderPauseMenu", nullptr);
    renderPauseMenu(game);
    break;
  case GameStateType::Exit:
    break;
  }
  EndDrawing();
}

std::vector<Texture2D> loadFrames(const char *fileNames[], int numFrames) {
  std::vector<Texture2D> frames;
  frames.reserve(numFrames);
  for (int i = 0; i < numFrames; i++) {
    LOG_DEBUG("%s", fileNames[i]);
    Texture2D texture = LoadTexture(fileNames[i]);
    if (texture.id == 0) {
      LOG_ERROR("Failed to load texture for file: ", fileNames[i]);
    }
    frames.push_back(texture);
  }
  return frames;
}

Animation *CreateAnimation(std::vector<Texture2D> frames, float frameSpeed) {
  Animation *animation = new Animation{};
  animation->frames = std::move(frames);
  animation->frameSpeed = frameSpeed;
  if (animation == nullptr) {
    LOG_ERROR("Allocation of animation failed!", nullptr);
  }
  return animation;
}

int getCharAnimationFiles(const char *file_names[], int num_frames, PlayerState state,
                          int char_id) {
  for (int i = 0; i < num_frames; i++) {
    char *path = new char[256];
    char str[] = "assets/characters/%02d/%s/sprite_%03d.png";
    switch (state) {
    case PlayerState::Spawn:
      sprintf(path, str, char_id, "spawn", i);
      file_names[i] = path;
      continue;
    case PlayerState::Idle:
      sprintf(path, str, char_id, "idle", i);
      file_names[i] = path;
      continue;
    case PlayerState::Walking:
      sprintf(path, str, char_id, "walking", i);
      file_names[i] = path;
      continue;
    case PlayerState::Death:
      sprintf(path, str, char_id, "death", i);
      file_names[i] = path;
      continue;
    default:
      return 1;
      break;
    }
  }
  return 0;
}

void loadCharacterAnimation(Player *player, PlayerState state, int char_id) {
  int num_frames;
  switch (state) {
  case PlayerState::Spawn:
    num_frames = kCharacterSpawnFramesNum;
    break;
  case PlayerState::Idle:
    num_frames = kCharacterIdleFramesNum;
    break;
  case PlayerState::Walking:
    num_frames = kCharacterWalkingFramesNum;
    break;
  case PlayerState::Death:
    num_frames = kCharacterDeathFramesNum;
    break;
  default:
    num_frames = 0;
    break;
  }
  const char **file_names = new const char *[num_frames];
  getCharAnimationFiles(file_names, num_frames, state, char_id);
  auto frames = loadFrames(file_names, num_frames);
  player->animation[static_cast<int>(state)].reset(CreateAnimation(std::move(frames), 0.1f));
  for (int i = 0; i < num_frames; i++) {
    delete[] file_names[i];
  }
  delete[] file_names;
}

void SetCharAnimationSet(int char_id, Player *player) {
  for (int i = 0; i < kPlayerStateCount; i++) {
    loadCharacterAnimation(player, static_cast<PlayerState>(i), char_id);
  }
}

void updateAnimation(Animation *animation, float deltaTime) {
  LOG_DEBUG("updateAnimation", nullptr);
  animation->elapsedTime += deltaTime;
  if (animation->elapsedTime >= animation->frameSpeed) {
    animation->elapsedTime -= animation->frameSpeed;
    animation->currentFrame =
        (animation->currentFrame + 1) % animation->frames.size();
  }
}

void drawAnimationV(Animation *animation, Vector2 position, Color color) {
  LOG_DEBUG("drawAnimationV: %i", animation->frames[animation->currentFrame].id);
  DrawTextureV(animation->frames[animation->currentFrame], position, color);
}

void drawAnimationPro(Animation *animation, Rectangle sourceRec,
                      Rectangle destRec, Vector2 origin, float rotation,
                      Color color) {
  LOG_DEBUG("drawAnimationPro: %i", animation->frames[animation->currentFrame].id);
  DrawTexturePro(animation->frames[animation->currentFrame], sourceRec, destRec,
                 origin, rotation, color);
}

void drawCenteredText(const char *text, int pos_y, int font_size, Color color) {
  int screenWidth = GetScreenWidth();
  DrawText(text, screenWidth / 2 - MeasureText(text, font_size) / 2, pos_y,
           font_size, color);
}

void renderMainMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  int maxTileWidth = screenWidth / kTileSize;
  int maxTileHeight = screenHeight / kTileSize;

  DrawText(game->title.c_str(),
           screenWidth / 2 - MeasureText(game->title.c_str(), fontSize * 2) / 2,
           kTileSize * 2, fontSize * 2, WHITE);

  DrawRectangleLines(maxTileWidth / 2 * kTileSize - kTileSize * 6, kTileSize,
                     kTileSize * 12, kTileSize * 4, WHITE);

  for (int i = 0; i < kMenuOptions; i++) {
    int textWidth = MeasureText(game->mainMenu->options[i].c_str(), fontSize);
    int x = screenWidth / 2 - textWidth / 2;
    int y = (kTileSize) * 6 + (i * kTileSize * 2);

    if (i == game->mainMenu->selectedOption) {
      DrawText(game->mainMenu->options[i].c_str(), x, y, fontSize, RED);
      DrawLine(x, y + kTileSize, x + textWidth, y + kTileSize, RED);
    } else {
      DrawText(game->mainMenu->options[i].c_str(), x, y, fontSize, WHITE);
    }
  }

  DrawText("Erstellt von Stefan Dillenburg", kTileSize,
           screenHeight - kTileSize, kTileSize / 2, WHITE);
  DrawText("Powered by", (maxTileWidth - 7) * kTileSize,
           screenHeight - kTileSize, kTileSize / 4, WHITE);
  DrawTexture(raylibLogo, (maxTileWidth - 5) * kTileSize,
              (maxTileHeight - 4) * kTileSize, WHITE);
}

void renderCharSelectMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  // Draw Title
  DrawText(game->charSelectMenu->title.c_str(),
           screenWidth / 2 -
               MeasureText(game->charSelectMenu->title.c_str(), fontSize) / 2,
           kTileSize * 2, fontSize, WHITE);
  // Draw Character
  int select = game->charSelectMenu->selectedOption;
  Texture2D texture = game->charSelectMenu->charTexture[select];
  Rectangle source = {0, 0, 64, 64};
  DrawTexturePro(
      texture, source,
      (Rectangle){static_cast<float>(screenWidth / 2 - kTileSize * 6 / 2),
                  kTileSize * 4, kTileSize * 6, kTileSize * 6},
      (Vector2){0, 0}, 0, WHITE);
  // Draw Character Frame
  DrawRectangleLines(screenWidth / 2 - kTileSize * 6 / 2, kTileSize * 4,
                     kTileSize * 6, kTileSize * 6, WHITE);
  // Selection Counter
  char selectCoutner[4];
  sprintf(selectCoutner, "%01d", select + 1);
  DrawText(selectCoutner,
           screenWidth / 2 - MeasureText(selectCoutner, fontSize) / 2,
           kTileSize * 11 - 16, fontSize, WHITE);
  // Draw Arrows
  Rectangle left = {0, 0, 32, 32};
  Rectangle dest = {
      static_cast<float>(screenWidth / 2 - kTileSize * 2 / 2 - kTileSize - 8),
      kTileSize * 10, kTileSize * 2, kTileSize * 2};
  DrawTexturePro(arrowTexture, left, dest, (Vector2){0, 0}, 0, WHITE);
  Rectangle right = {0, 0, -32, 32};
  dest = (Rectangle){
      static_cast<float>(screenWidth / 2 - kTileSize * 2 / 2 + kTileSize + 8),
      kTileSize * 10, kTileSize * 2, kTileSize * 2};
  DrawTexturePro(arrowTexture, right, dest, (Vector2){0, 0}, 0, WHITE);
}

void renderRunningCountdown(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  int maxTileHeight = GetScreenHeight() % kTileSize;
  Color overlayColor = Fade(kBackgroundColor, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  drawCenteredText("Countdown", kTileSize, fontSize, WHITE);
  char timerText[100];
  sprintf(timerText, "%.0f", game->countdown);
  drawCenteredText(timerText, maxTileHeight / 2 * kTileSize, fontSize * 4, RED);
}

void renderRunning(const Game *game) {
  LOG_DEBUG("renderRunning: renderStats", nullptr);
  renderStats(game);
  LOG_DEBUG("renderRunning: renderMap", nullptr);
  renderMap(game);
  LOG_DEBUG("renderRunning: renderItems", nullptr);
  renderItems(game);
  LOG_DEBUG("renderRunning: renderBombs", nullptr);
  renderBombs(game);
  LOG_DEBUG("renderRunning: renderPlayer", nullptr);
  renderPlayer(game);
  LOG_DEBUG("renderRunning: renderExplosions", nullptr);
  renderExplosions(game);
}

void renderPauseMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(kBackgroundColor, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  drawCenteredText(game->pauseMenu->title.c_str(), kTileSize, fontSize, WHITE);
}

Vector2 getGridOffset() {
  return (Vector2){
      static_cast<float>(GetScreenWidth() / 2 - (kTileSize * kGridWidth) / 2),
      static_cast<float>(GetScreenHeight() / 2 -
                         (kTileSize * kGridHeight) / 2)};
}

void renderMap(const Game *game) {
  Vector2 offset = getGridOffset();
  DrawTextureV(mapTexture, offset, WHITE);
}

void renderStats(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();

  // FPS
  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           screenWidth - MeasureText(fpsText, fontSize / 2) - kTileSize,
           kTileSize, fontSize / 2, WHITE);

  DrawRectangleLines(kTileSize, kTileSize, kTileSize * 7, kTileSize * 5, WHITE);

  // Player Look
  for (int i = 0; i < kMaxPlayers; i++) {
    auto *anim = game->player[i]->animation[static_cast<int>(PlayerState::Idle)].get();
    if (!anim || anim->frames.empty()) continue;
    Texture2D characterTexture = anim->frames[0];
    Rectangle source = (Rectangle){12, 12, 36, 36};
    if (i == 0) {
      DrawTexturePro(characterTexture, source,
                     (Rectangle){kTileSize * 2 - 8, kTileSize * 2 - 8,
                                 kTileSize, kTileSize},
                     (Vector2){0, 0}, 0, WHITE);
    } else {
      float x = screenWidth - kTileSize * i - 8;
      float y = kTileSize * 2 - 8;
      DrawTexturePro(characterTexture, source,
                     (Rectangle){x, y, kTileSize, kTileSize}, (Vector2){0, 0},
                     0, WHITE);
      if (!game->player[i]->isAlive) {
        DrawLine(x, y, x + kTileSize, y + kTileSize, RED);
      }
    }
  }

  // Speed
  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f", game->player[0]->speed);
  DrawText(speedText, kTileSize * 2, kTileSize * 3, fontSize / 2, WHITE);

  // Bombs
  DrawText("Bomben:", kTileSize * 2, kTileSize * 4, fontSize / 2, WHITE);
  int textWidth = MeasureText("Bomben:", fontSize / 2);
  for (int i = 0; i < game->player[0]->bombs; i++) {
    bool slotEmpty = i >= static_cast<int>(game->player[0]->bombList.size()) ||
                     game->player[0]->bombList[i] == nullptr;
    if (slotEmpty) {
      DrawTextureV(bombTexture,
                   (Vector2){static_cast<float>(textWidth + kTileSize * 2 + (kTileSize * i) + 8),
                             kTileSize * 4 - 8},
                   WHITE);
    }
  }

  // Blast-Radius
  char blastRadiusText[100];
  sprintf(blastRadiusText, "Explosionsradius: %i",
          game->player[0]->blastRadius);
  DrawText(blastRadiusText, kTileSize * 2, kTileSize * 5, fontSize / 2, WHITE);

  // Controls
  const char *explainControlsText =
      "[W],[S],[A],[D] Bewegen\n[SPACE] Bombe\n[P] Pause\n";
  DrawText(explainControlsText, kTileSize, GetScreenHeight() - kTileSize * 2,
           fontSize / 4, WHITE);
}

bool isLastFrame(Animation *animation) {
  if (static_cast<int>(animation->frames.size()) - 1 == animation->currentFrame) {
    return true;
  }
  return false;
}

void renderPlayerAnimation(Animation *animation, Rectangle source,
                           Rectangle dest, float deltaTime) {
  drawAnimationPro(animation, source, dest, (Vector2){0, 0}, 0, WHITE);
  updateAnimation(animation, deltaTime);
}

void renderPlayer(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->player[i].get();
      Vector2 position = {
        Lerp(player->entity.position.x, player->entity.targetPosition.x,
             player->entity.progress),
        Lerp(player->entity.position.y, player->entity.targetPosition.y,
             player->entity.progress)};
    position = (Vector2){kTileSize * position.x + offset.x,
                         kTileSize * position.y + offset.y - 8};
    Rectangle source;
    if (player->entity.facing == Direction::East) {
      source = (Rectangle){12, 12, 36, 36};
    } else {
      source = (Rectangle){12, 12, -36, 36};
    }
    LOG_DEBUG("player->state: %i:%i", i, player->state);
    Rectangle dest = {position.x, position.y, kTileSize, kTileSize};
    switch (player->state) {
case PlayerState::Spawn: {
      auto *anim = player->animation[static_cast<int>(PlayerState::Spawn)].get();
      if (anim && !isLastFrame(anim)) {
        renderPlayerAnimation(anim, source, dest, game->deltaTime);
      }
      break;
    }
    case PlayerState::Idle: {
      auto *anim = player->animation[static_cast<int>(PlayerState::Idle)].get();
      if (anim) renderPlayerAnimation(anim, source, dest, game->deltaTime);
      break;
    }
    case PlayerState::Walking: {
      auto *anim = player->animation[static_cast<int>(PlayerState::Walking)].get();
      if (anim) renderPlayerAnimation(anim, source, dest, game->deltaTime);
      break;
    }
    case PlayerState::Death: {
      auto *anim = player->animation[static_cast<int>(PlayerState::Death)].get();
      if (anim && static_cast<int>(anim->frames.size()) - 1 != anim->currentFrame) {
        renderPlayerAnimation(anim, source, dest, game->deltaTime);
      }
      break;
    }
    default:
      break;
    }
  }
}

void renderBombs(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->player[i].get();
    for (auto &bomb : player->bombList) {
      if (bomb != nullptr && bomb->endTime != 0) {
        Vector2 position = {kTileSize * bomb->entity.position.x + offset.x,
                            kTileSize * bomb->entity.position.y + offset.y};
        DrawTextureV(bombTexture, position, WHITE);
        position.y -= 8;
        drawAnimationV(bomb->animation, position, WHITE);
        updateAnimation(bomb->animation, game->deltaTime);
      }
    }
  }
}

void renderExplosions(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->player[i].get();
    for (auto &bomb : player->bombList) {
      if (bomb != nullptr && bomb->endTime == 0) {
        for (int k = 0; k < kDirectionCount; k++) {
          if (bomb->explosion[k] != nullptr) {
            Explosion *explosion = bomb->explosion[k].get();
            float x, y;
            x = Lerp(explosion->entity.position.x,
                     explosion->entity.targetPosition.x,
                     explosion->entity.progress);
            y = Lerp(explosion->entity.position.y,
                     explosion->entity.targetPosition.y,
                     explosion->entity.progress);
            Vector2 v = {kTileSize * x + offset.x, kTileSize * y + offset.y};
            Rectangle rec;
            int rotation = 0;
            Vector2 origin = {0, 0};
            switch (static_cast<Direction>(k)) {
            case Direction::North:
              rec = (Rectangle){0, 0, 32, 32};
              rotation = 270;
              origin = (Vector2){32, 0};
              break;
            case Direction::East:
              rec = (Rectangle){0, 0, 32, 32};
              break;
            case Direction::South:
              rec = (Rectangle){0, 0, 32, 32};
              rotation = 90;
              origin = (Vector2){0, 32};
              break;
            case Direction::West:
              rec = (Rectangle){0, 0, -32, 32};
              break;
            }
            drawAnimationPro(explosion->animation, rec,
                             (Rectangle){v.x, v.y, kTileSize, kTileSize},
                             origin, rotation, WHITE);
            updateAnimation(explosion->animation, game->deltaTime);
          }
        }
      }
    }
  }
}

void renderItems(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int x = 0; x < kGridWidth; x++) {
    for (int y = 0; y < kGridHeight; y++) {
      Cell cell = game->grid[x][y];
      Vector2 position = {kTileSize * x + offset.x, kTileSize * y + offset.y};
      switch (cell.type) {
      case CellType::Destructible:
        DrawTextureV(crateTexture, position, WHITE);
        break;
      case CellType::PowerUp:
        drawAnimationV(starAnimation, position, WHITE);
        updateAnimation(starAnimation, game->deltaTime);
      default:
        break;
      }
    }
  }
}
