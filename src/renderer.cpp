#include "renderer.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "network_manager.hpp"
#include "animation.hpp"
#include "util.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
  LOG_DEBUG("Load static textures");
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

  LOG_DEBUG("Load static frames");
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
  LOG_DEBUG("Render");
  BeginDrawing();
  ClearBackground(kBackgroundColor);
  switch (game->state) {
  case GameStateType::MainMenu:
    LOG_DEBUG("Render: renderMainMenu");
    renderMainMenu(game);
    break;
  case GameStateType::HostGameMenu:
    LOG_DEBUG("Render: renderHostGameMenu");
    renderHostGameMenu(game);
    break;
  case GameStateType::JoinGameMenu:
    LOG_DEBUG("Render: renderJoinGameMenu");
    renderJoinGameMenu(game);
    break;
  case GameStateType::RunningCountdown:
    LOG_DEBUG("Render: renderRunning");
    renderRunning(game);
    LOG_DEBUG("Render: renderRunningCountdown");
    renderRunningCountdown(game);
    break;
  case GameStateType::CharSelectMenu:
    LOG_DEBUG("Render: renderCharSelectMenu");
    renderCharSelectMenu(game);
    break;
  case GameStateType::ClientCharSelect:
    LOG_DEBUG("Render: renderCharSelectMenu");
    renderCharSelectMenu(game);
    break;
  case GameStateType::Running:
    LOG_DEBUG("Render: renderRunning");
    renderRunning(game);
    break;
  case GameStateType::ClientRunning:
    LOG_DEBUG("Render: renderRunning");
    renderRunning(game);
    if (game->clientCountdown > 0) {
      LOG_DEBUG("Render: renderRunningCountdown");
      renderRunningCountdown(game);
    }
    break;
  case GameStateType::GameOver:
    LOG_DEBUG("Render: renderRunning");
    renderRunning(game);
    LOG_DEBUG("Render: renderGameOver");
    renderGameOver(game);
    break;
  case GameStateType::PauseMenu:
    LOG_DEBUG("Render: renderRunning");
    renderRunning(game);
    LOG_DEBUG("Render: renderPauseMenu");
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

std::vector<std::string> Renderer::getCharAnimationFiles(PlayerState state,
                                                          int char_id,
                                                          int num_frames) {
  std::vector<std::string> file_names;
  file_names.reserve(num_frames);
  for (int i = 0; i < num_frames; i++) {
    char buf[256];
    const char *anim_name;
    switch (state) {
    case PlayerState::Spawn:  anim_name = "spawn"; break;
    case PlayerState::Idle:   anim_name = "idle"; break;
    case PlayerState::Walking: anim_name = "walking"; break;
    case PlayerState::Death:  anim_name = "death"; break;
    default:
      return {};
    }
    snprintf(buf, sizeof(buf), "assets/characters/%02d/%s/sprite_%03d.png",
             char_id, anim_name, i);
    file_names.emplace_back(buf);
  }
  return file_names;
}

void Renderer::loadCharacterAnimation(Player *player, PlayerState state,
                                       int char_id) {
  int num_frames;
  switch (state) {
  case PlayerState::Spawn:  num_frames = kCharacterSpawnFramesNum; break;
  case PlayerState::Idle:   num_frames = kCharacterIdleFramesNum; break;
  case PlayerState::Walking: num_frames = kCharacterWalkingFramesNum; break;
  case PlayerState::Death:  num_frames = kCharacterDeathFramesNum; break;
  default: return;
  }
  auto file_names = getCharAnimationFiles(state, char_id, num_frames);
  std::vector<Texture2D> frames;
  frames.reserve(num_frames);
  for (const auto &fn : file_names) {
    Texture2D texture = LoadTexture(fn.c_str());
    if (texture.id == 0) {
      LOG_ERROR("Failed to load texture: %s", fn.c_str());
    }
    frames.push_back(texture);
  }
  player->setAnimation(
      state, std::make_unique<Animation>(std::move(frames), 0.1f));
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

void Renderer::renderHostGameMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  drawCenteredText("LAN Host", kTileSize * 2, fontSize * 2, WHITE);

  char buf[128];
  sprintf(buf, "Server Port: %d", kServerPort);
  drawCenteredText(buf, kTileSize * 5, fontSize, WHITE);

  sprintf(buf, "Spieleranzahl: %d", game->hostPlayerCount);
  drawCenteredText(buf, kTileSize * 7, fontSize, WHITE);
  drawCenteredText("[<]  [>]", kTileSize * 8, fontSize / 2, GRAY);

  sprintf(buf, "Verbunden: %d / %d", game->numConnectedClients,
          game->hostPlayerCount - 1);
  drawCenteredText(buf, kTileSize * 10, fontSize, WHITE);

  if (game->numConnectedClients >= game->hostPlayerCount - 1) {
    drawCenteredText("ENTER zum Starten", kTileSize * 12, fontSize, GREEN);
  } else {
    drawCenteredText("Warte auf Spieler...", kTileSize * 12, fontSize, GRAY);
  }

  DrawText("ESC - Zurueck", kTileSize, screenHeight - kTileSize, fontSize / 2,
           WHITE);
}

void Renderer::renderJoinGameMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  drawCenteredText("LAN Beitreten", kTileSize * 2, fontSize * 2, WHITE);

  drawCenteredText("IP-Adresse:", kTileSize * 5, fontSize, WHITE);

  char ipDisplay[32];
  sprintf(ipDisplay, "%s", game->serverIP);
  if (strlen(game->serverIP) == 0) {
    sprintf(ipDisplay, " ");
  }
  if (game->isConnecting) {
    drawCenteredText("Verbinde...", kTileSize * 7, fontSize, YELLOW);
  } else if (game->connectionFailed) {
    drawCenteredText("Verbindung fehlgeschlagen!", kTileSize * 7, fontSize, RED);
    drawCenteredText("ESC - Zurueck", kTileSize * 8, fontSize / 2, GRAY);
  } else {
    drawCenteredText(ipDisplay, kTileSize * 7, fontSize, SKYBLUE);
    drawCenteredText("ENTER - Verbinden", kTileSize * 9, fontSize / 2, GRAY);
  }

  DrawText("ESC - Zurueck", kTileSize, screenHeight - kTileSize, fontSize / 2,
           WHITE);
}

void Renderer::renderCharSelectMenu(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();

  const char *title;
  char titleBuf[64];
  if (game->state == GameStateType::ClientCharSelect) {
    if (game->currentSelectingPlayer > 0) {
      sprintf(titleBuf, "Deine Wahl (Spieler %d)", game->currentSelectingPlayer);
    } else {
      sprintf(titleBuf, "Warte auf deine Auswahl...");
    }
    title = titleBuf;
  } else if (game->isServer && game->currentSelectingPlayer > 0) {
    sprintf(titleBuf, "Warte auf Spieler %d...", game->currentSelectingPlayer);
    title = titleBuf;
  } else {
    title = game->charSelectMenu->title.c_str();
  }
  DrawText(title,
           screenWidth / 2 -
               MeasureText(title, fontSize) / 2,
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
  int maxTileHeight = GetScreenHeight() / kTileSize;
  Color overlayColor = Fade(kBackgroundColor, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  drawCenteredText("Countdown", kTileSize, fontSize, WHITE);
  char timerText[100];
  float cd = (game->state == GameStateType::ClientRunning)
                 ? game->clientCountdown
                 : game->countdown;
  sprintf(timerText, "%.0f", cd);
  drawCenteredText(timerText, maxTileHeight / 2 * kTileSize, fontSize * 4, RED);
}

void Renderer::renderRunning(const Game *game) {
  LOG_DEBUG("renderRunning: renderStats");
  renderStats(game);
  LOG_DEBUG("renderRunning: renderMap");
  renderMap(game);
  starAnimation_->update(game->deltaTime);
  LOG_DEBUG("renderRunning: renderItems");
  renderItems(game);
  LOG_DEBUG("renderRunning: renderBombs");
  renderBombs(game);
  LOG_DEBUG("renderRunning: renderPlayer");
  renderPlayer(game);
  LOG_DEBUG("renderRunning: renderExplosions");
  renderExplosions(game);
}

void Renderer::renderGameOver(const Game *game) {
  int fontSize = kTileSize;
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  Color overlayColor = Fade(kBackgroundColor, 0.6f);
  DrawRectangle(0, 0, screenWidth, screenHeight, overlayColor);

  if (game->winnerId >= 0) {
    char buf[64];
    sprintf(buf, "Spieler %d gewinnt!", game->winnerId + 1);
    drawCenteredText(buf, kTileSize * 3, fontSize * 2, GREEN);
  } else if (game->winnerId == -2) {
    drawCenteredText("Verbindung verloren!", kTileSize * 3, fontSize * 2, RED);
  } else {
    drawCenteredText("Unentschieden!", kTileSize * 3, fontSize * 2, YELLOW);
  }

  drawCenteredText("ENTER - Hauptmenue", kTileSize * 10, fontSize, WHITE);
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
  int statsPlayerId = game->isClient ? game->clientPlayerId : 0;
  if (statsPlayerId < 0) statsPlayerId = 0;

  char fpsText[100];
  sprintf(fpsText, "FPS: %i", GetFPS());
  DrawText(fpsText,
           screenWidth - MeasureText(fpsText, fontSize / 2) - kTileSize,
           kTileSize, fontSize / 2, WHITE);

  DrawRectangleLines(kTileSize, kTileSize, kTileSize * 7, kTileSize * 5, WHITE);

  int rightIdx = 0;
  for (int i = 0; i < kMaxPlayers; i++) {
    auto *anim =
        game->playerManager.getPlayer(i)->getAnimation(PlayerState::Idle);
    if (!anim || anim->frames().empty()) continue;
    Texture2D characterTexture = anim->frames()[0];
    Rectangle source = (Rectangle){12, 12, 36, 36};
    if (i == statsPlayerId) {
      DrawTexturePro(characterTexture, source,
                     (Rectangle){kTileSize * 2 - 8, kTileSize * 2 - 8,
                                 kTileSize, kTileSize},
                     (Vector2){0, 0}, 0, WHITE);
    } else {
      float x = screenWidth - kTileSize * (rightIdx + 1) - 8;
      float y = kTileSize * 2 - 8;
      DrawTexturePro(characterTexture, source,
                     (Rectangle){x, y, kTileSize, kTileSize}, (Vector2){0, 0},
                     0, WHITE);
      if (!game->playerManager.getPlayer(i)->isAlive()) {
        DrawLine(x, y, x + kTileSize, y + kTileSize, RED);
      }
      rightIdx++;
    }
  }

  const Player *statsPlayer = game->playerManager.getPlayer(statsPlayerId);

  char speedText[100];
  sprintf(speedText, "Geschwindigkeit: %.0f", statsPlayer->getSpeed());
  DrawText(speedText, kTileSize * 2, kTileSize * 3, fontSize / 2, WHITE);

  DrawText("Bomben:", kTileSize * 2, kTileSize * 4, fontSize / 2, WHITE);
  int textWidth = MeasureText("Bomben:", fontSize / 2);
  int bombCount = statsPlayer->getBombs();
  int activeBombs = game->bombManager.getBombCountForPlayer(statsPlayerId);
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
          statsPlayer->getBlastRadius());
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
        break;
      default:
        break;
      }
    }
  }
}
