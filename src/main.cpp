#include "game.hpp"
#include "logger.hpp"
#include "network_manager.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <cstdlib>

// Fenster Einstellungen
constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;
constexpr const char *windowTitle = "Bomberman";
constexpr int windowFPS = 60;

int main() {
  // V-Sync vor InitWindow setzen (Raylib Docs: SetConfigFlags muss vor InitWindow!)
  SetConfigFlags(FLAG_VSYNC_HINT);
  // Fenster erstellen und Einstellungen setzen
  InitWindow(windowWidth, windowHeight, windowTitle);
  SetTargetFPS(windowFPS);

  // Entferne Tastenbelegung vom ESC-Key zum schließen des Spiels
  SetExitKey(KEY_NULL);

  if (getenv("DEBUG")) {
    Logger::setLevel(LogLevel::Debug);
    LOG_DEBUG("Set log level to debbuging");
    // SetTargetFPS(5);
  }

  if (!NetworkManager::init()) {
    LOG_ERROR("Failed to initialize ENet");
    CloseWindow();
    return 1;
  }

  LOG_DEBUG("InitGame");
  Game *game = InitGame();
  LOG_DEBUG("InitRenderer");
  Renderer::init(game);
  LOG_DEBUG("GameLoop");
  GameLoop(game);
  LOG_DEBUG("DestroyGame");
  DestroyGame(game);
  LOG_DEBUG("UnloadRenderer");
  Renderer::unload();
  LOG_DEBUG("CloseWindow");
  CloseWindow();
  NetworkManager::deinit();
  return 0;
}
