#include "renderer.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "animation.hpp"
#include "util.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>

constexpr int kTileSize = 32;
constexpr Color kBackgroundColor = {30, 30, 30, 255};

// File pointer arrays
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

// Static member definitions
std::vector<Texture2D> Renderer::bombSparkFrames_;
std::vector<Texture2D> Renderer::explosionBlastFrames_;
std::vector<Texture2D> Renderer::starFrames_;
std::unique_ptr<Animation> Renderer::starAnimation_;

Texture2D Renderer::raylibLogo_{};
Texture2D Renderer::arrowTexture_{};
Texture2D Renderer::mapTexture_{};
Texture2D Renderer::crateTexture_{};
Texture2D Renderer::bombTexture_{};

void Renderer::init(Game *game) {
  LOG_DEBUG("Load static textures", nullptr);
  raylibLogo_ = LoadTexture("assets/raylib_logo.png");
  arrowTexture_ = LoadTexture("assets/ui/arrow.png");
  mapTexture_ = LoadTexture("assets/map.png");
  crateTexture_ = LoadTexture("assets/items/crate.png");
  bombTexture_ = LoadTexture("assets/items/bomb.png");

  for (int i = 0; i < kCharacters; i++) {
    char path[256];
    sprintf(path, "assets/characters/%02d/idle/sprite_000.png", i);
    game->charSelectMenu->charTexture[i] = LoadTexture(path);
  }

  LOG_DEBUG("Load static frames", nullptr);
  starFrames_ = loadFrames(starFiles, kStarFramesNum);
  bombSparkFrames_ = loadFrames(bombSparkFiles, kBombSparkFramesNum);
  explosionBlastFrames_ =
      loadFrames(explosionBlastFiles, kExplosionBlastFramesNum);

  starAnimation_ = std::make_unique<Animation>(SharedFramesTag{}, starFrames_, 0.1f);
}

void Renderer::unload() {
  UnloadTexture(raylibLogo_);
  UnloadTexture(arrowTexture_);
  UnloadTexture(mapTexture_);
  UnloadTexture(crateTexture_);
  UnloadTexture(bombTexture_);
  for (auto &f : starFrames_) UnloadTexture(f);
  for (auto &f : bombSparkFrames_) UnloadTexture(f);
  for (auto &f : explosionBlastFrames_) UnloadTexture(f);
  starAnimation_.reset();
}

void Renderer::render(const Game *game) {
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

const std::vector<Texture2D> &Renderer::getBombSparkFrames() {
  return bombSparkFrames_;
}

const std::vector<Texture2D> &Renderer::getExplosionBlastFrames() {
  return explosionBlastFrames_;
}

std::vector<Texture2D> Renderer::loadFrames(const char *fileNames[],
                                             int numFrames) {
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

int Renderer::getCharAnimationFiles(const char *file_names[], int num_frames,
                                     PlayerState state, int char_id) {
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
    }
  }
  return 0;
}

void Renderer::loadCharacterAnimation(Player *player, PlayerState state,
                                       int char_id) {
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
  player->setAnimation(
      state, std::make_unique<Animation>(std::move(frames), 0.1f));
  for (int i = 0; i < num_frames; i++) {
    delete[] file_names[i];
  }
  delete[] file_names;
}

void Renderer::setCharAnimationSet(int char_id, Player *player) {
  for (int i = 0; i < kPlayerStateCount; i++) {
    loadCharacterAnimation(player, static_cast<PlayerState>(i), char_id);
  }
}

void Renderer::drawCenteredText(const char *text, int pos_y, int font_size,
                                 Color color) {
  int screenWidth = GetScreenWidth();
  DrawText(text, screenWidth / 2 - MeasureText(text, font_size) / 2, pos_y,
           font_size, color);
}

void Renderer::renderMainMenu(const Game *game) {
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
    int y = (kTileSize)*6 + (i * kTileSize * 2);

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
  DrawTexture(raylibLogo_, (maxTileWidth - 5) * kTileSize,
              (maxTileHeight - 4) * kTileSize, WHITE);
}

void Renderer::renderCharSelectMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  DrawText(game->charSelectMenu->title.c_str(),
           screenWidth / 2 -
               MeasureText(game->charSelectMenu->title.c_str(), fontSize) / 2,
           kTileSize * 2, fontSize, WHITE);
  int select = game->charSelectMenu->selectedOption;
  Texture2D texture = game->charSelectMenu->charTexture[select];
  Rectangle source = {0, 0, 64, 64};
  DrawTexturePro(
      texture, source,
      (Rectangle){static_cast<float>(screenWidth / 2 - kTileSize * 6 / 2),
                  kTileSize * 4, kTileSize * 6, kTileSize * 6},
      (Vector2){0, 0}, 0, WHITE);
  DrawRectangleLines(screenWidth / 2 - kTileSize * 6 / 2, kTileSize * 4,
                     kTileSize * 6, kTileSize * 6, WHITE);
  char selectCoutner[4];
  sprintf(selectCoutner, "%01d", select + 1);
  DrawText(selectCoutner,
           screenWidth / 2 - MeasureText(selectCoutner, fontSize) / 2,
           kTileSize * 11 - 16, fontSize, WHITE);
  Rectangle left = {0, 0, 32, 32};
  Rectangle dest = {
      static_cast<float>(screenWidth / 2 - kTileSize * 2 / 2 - kTileSize - 8),
      kTileSize * 10, kTileSize * 2, kTileSize * 2};
  DrawTexturePro(arrowTexture_, left, dest, (Vector2){0, 0}, 0, WHITE);
  Rectangle right = {0, 0, -32, 32};
  dest = (Rectangle){
      static_cast<float>(screenWidth / 2 - kTileSize * 2 / 2 + kTileSize + 8),
      kTileSize * 10, kTileSize * 2, kTileSize * 2};
  DrawTexturePro(arrowTexture_, right, dest, (Vector2){0, 0}, 0, WHITE);
}

void Renderer::renderRunningCountdown(const Game *game) {
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

void Renderer::renderRunning(const Game *game) {
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

void Renderer::renderPauseMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(kBackgroundColor, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  drawCenteredText(game->pauseMenu->title.c_str(), kTileSize, fontSize, WHITE);
}

Vector2 Renderer::getGridOffset() {
  return (Vector2){
      static_cast<float>(GetScreenWidth() / 2 - (kTileSize * kGridWidth) / 2),
      static_cast<float>(GetScreenHeight() / 2 -
                         (kTileSize * kGridHeight) / 2)};
}

void Renderer::renderMap(const Game *game) {
  Vector2 offset = getGridOffset();
  DrawTextureV(mapTexture_, offset, WHITE);
}

void Renderer::renderStats(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();

  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           screenWidth - MeasureText(fpsText, fontSize / 2) - kTileSize,
           kTileSize, fontSize / 2, WHITE);

  DrawRectangleLines(kTileSize, kTileSize, kTileSize * 7, kTileSize * 5, WHITE);

  for (int i = 0; i < kMaxPlayers; i++) {
    auto *anim =
        game->playerManager.getPlayer(i)->getAnimation(PlayerState::Idle);
    if (!anim || anim->frames().empty()) continue;
    Texture2D characterTexture = anim->frames()[0];
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
      if (!game->playerManager.getPlayer(i)->isAlive()) {
        DrawLine(x, y, x + kTileSize, y + kTileSize, RED);
      }
    }
  }

  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f",
          game->playerManager.getPlayer(0)->getSpeed());
  DrawText(speedText, kTileSize * 2, kTileSize * 3, fontSize / 2, WHITE);

  DrawText("Bomben:", kTileSize * 2, kTileSize * 4, fontSize / 2, WHITE);
  int textWidth = MeasureText("Bomben:", fontSize / 2);
  int bombCount = game->playerManager.getPlayer(0)->getBombs();
  int activeBombs = game->bombManager.getBombCountForPlayer(0);
  for (int i = 0; i < bombCount; i++) {
    bool slotEmpty = i >= activeBombs;
    if (slotEmpty) {
      DrawTextureV(
          bombTexture_,
          (Vector2){static_cast<float>(textWidth + kTileSize * 2 +
                                       (kTileSize * i) + 8),
                    kTileSize * 4 - 8},
          WHITE);
    }
  }

  char blastRadiusText[100];
  sprintf(blastRadiusText, "Explosionsradius: %i",
          game->playerManager.getPlayer(0)->getBlastRadius());
  DrawText(blastRadiusText, kTileSize * 2, kTileSize * 5, fontSize / 2, WHITE);

  const char *explainControlsText =
      "[W],[S],[A],[D] Bewegen\n[SPACE] Bombe\n[P] Pause\n";
  DrawText(explainControlsText, kTileSize, GetScreenHeight() - kTileSize * 2,
           fontSize / 4, WHITE);
}

void Renderer::renderPlayerAnimation(Animation *animation, Rectangle source,
                                      Rectangle dest, float deltaTime) {
  animation->drawPro(source, dest, (Vector2){0, 0}, 0, WHITE);
  animation->update(deltaTime);
}

void Renderer::renderPlayer(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int i = 0; i < kMaxPlayers; i++) {
    const Player *player = game->playerManager.getPlayer(i);
    Vector2 position = {
        Lerp(player->getPosition().x, player->getTargetPosition().x,
             player->getProgress()),
        Lerp(player->getPosition().y, player->getTargetPosition().y,
             player->getProgress())};
    position = (Vector2){kTileSize * position.x + offset.x,
                         kTileSize * position.y + offset.y - 8};
    Rectangle source;
    if (player->getFacing() == Direction::East) {
      source = (Rectangle){12, 12, 36, 36};
    } else {
      source = (Rectangle){12, 12, -36, 36};
    }
    LOG_DEBUG("player->state: %i:%i", i, player->getState());
    Rectangle dest = {position.x, position.y, kTileSize, kTileSize};
    switch (player->getState()) {
    case PlayerState::Spawn: {
      auto *anim = player->getAnimation(PlayerState::Spawn);
      if (anim && !anim->isLastFrame()) {
        renderPlayerAnimation(anim, source, dest, game->deltaTime);
      }
      break;
    }
    case PlayerState::Idle: {
      auto *anim = player->getAnimation(PlayerState::Idle);
      if (anim) renderPlayerAnimation(anim, source, dest, game->deltaTime);
      break;
    }
    case PlayerState::Walking: {
      auto *anim = player->getAnimation(PlayerState::Walking);
      if (anim) renderPlayerAnimation(anim, source, dest, game->deltaTime);
      break;
    }
    case PlayerState::Death: {
      auto *anim = player->getAnimation(PlayerState::Death);
      if (anim && !anim->isLastFrame()) {
        renderPlayerAnimation(anim, source, dest, game->deltaTime);
      }
      break;
    }
    default:
      break;
    }
  }
}

void Renderer::renderBombs(const Game *game) {
  Vector2 offset = getGridOffset();
  for (auto &bomb : game->bombManager.getBombs()) {
    if (bomb != nullptr && bomb->getEndTime() != 0) {
      Vector2 position = {kTileSize * bomb->getPosition().x + offset.x,
                          kTileSize * bomb->getPosition().y + offset.y};
      DrawTextureV(bombTexture_, position, WHITE);
      position.y -= 8;
      bomb->getAnimation()->draw(position, WHITE);
      bomb->getAnimation()->update(game->deltaTime);
    }
  }
}

void Renderer::renderExplosions(const Game *game) {
  Vector2 offset = getGridOffset();
  for (auto &bomb : game->bombManager.getBombs()) {
    if (bomb != nullptr && bomb->getEndTime() == 0) {
      for (int k = 0; k < kDirectionCount; k++) {
        auto &explosion = bomb->getExplosions()[k];
        if (explosion != nullptr) {
          float x, y;
          x = Lerp(explosion->getOrigin().x, explosion->getTargetPosition().x,
                   explosion->getProgress());
          y = Lerp(explosion->getOrigin().y, explosion->getTargetPosition().y,
                   explosion->getProgress());
          Vector2 v = {kTileSize * x + offset.x, kTileSize * y + offset.y};
          Rectangle rec;
          int rotation = 0;
          Vector2 origin = {0, 0};
          switch (explosion->getDirection()) {
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
          explosion->getAnimation()->drawPro(
              rec, (Rectangle){v.x, v.y, kTileSize, kTileSize}, origin,
              rotation, WHITE);
          explosion->getAnimation()->update(game->deltaTime);
        }
      }
    }
  }
}

void Renderer::renderItems(const Game *game) {
  Vector2 offset = getGridOffset();
  for (int x = 0; x < kGridWidth; x++) {
    for (int y = 0; y < kGridHeight; y++) {
      Cell cell = game->grid.getCell({x, y});
      Vector2 position = {kTileSize * x + offset.x, kTileSize * y + offset.y};
      switch (cell.type) {
      case CellType::Destructible:
        DrawTextureV(crateTexture_, position, WHITE);
        break;
      case CellType::PowerUp:
        starAnimation_->draw(position, WHITE);
        starAnimation_->update(game->deltaTime);
      default:
        break;
      }
    }
  }
}
