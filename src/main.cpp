#include "game.hpp"
#include "log.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <cstdlib>

// Fenster Einstellungen
constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;
constexpr const char *windowTitle = "Bomberman";
constexpr int windowFPS = 60;

int main() {
  // Fenster erstellen und Einstellungen setzen
  InitWindow(windowWidth, windowHeight, windowTitle);
  SetTargetFPS(windowFPS);
  // Set V-Sync
  SetConfigFlags(FLAG_VSYNC_HINT);

  // Entferne Tastenbelegung vom ESC-Key zum schließen des Spiels
  SetExitKey(KEY_NULL);

  if (getenv("DEBUG")) {
    currentLogLevel = LogLevel::Debug;
    LOG_DEBUG("Set log level to debbuging", nullptr);
    // SetTargetFPS(5);
  }

  LOG_DEBUG("InitGame", nullptr);
  Game *game = InitGame();
  LOG_DEBUG("InitRenderer", nullptr);
  InitRenderer(game);
  LOG_DEBUG("GameLoop", nullptr);
  GameLoop(game);
  LOG_DEBUG("DestroyGame", nullptr);
  DestroyGame(game);
  LOG_DEBUG("UnloadRenderer", nullptr);
  UnloadRenderer();
  LOG_DEBUG("CloseWindow", nullptr);
  CloseWindow();
  return 0;
}
