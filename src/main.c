#include "game.h"
#include "log.h"
#include "renderer.h"
#include <raylib.h>
#include <stdlib.h>

// Fenster Einstellungen
static const int windowWidth = 1280;
static const int windowHeight = 720;
static const char *windowTitle = "Bomberman";
static const int windowFPS = 60;

int main() {
  // Fenster erstellen und Einstellungen setzen
  InitWindow(windowWidth, windowHeight, windowTitle);
  SetTargetFPS(windowFPS);

  // Entferne Tastenbelegung vom ESC-Key zum schließen des Spiels
  SetExitKey(KEY_NULL);

  if (getenv("DEBUG")) {
    currentLogLevel = LOG_LEVEL_DEBUG;
    LOG_DEBUG("Set log level to debbuging", NULL);
    SetTargetFPS(5);
  }

  LOG_DEBUG("InitGame", NULL);
  Game *game = InitGame();
  LOG_DEBUG("InitRenderer", NULL);
  InitRenderer(game);
  LOG_DEBUG("GameLoop", NULL);
  GameLoop();
  LOG_DEBUG("CloseWindow", NULL);
  CloseWindow();
  return 0;
}
