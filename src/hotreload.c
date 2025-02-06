#include "hotreload.h"
#include "game.h"
#include "renderer.h"
#include <raylib.h>

// Fenster Einstellungen
static const int windowWidth = 800;
static const int windowHeight = 600;
static const char *windowTitle = "Bomberman";
static const int windowFPS = 60;

GameStateType currentState;
Game *game;

void Init() {
  // Fenster erstellen und Einstellungen setzen
  InitWindow(windowWidth, windowHeight, windowTitle);
  SetTargetFPS(windowFPS);

  // Entferne Tastenbelegung vom ESC-Key zum schließen des Spiels
  SetExitKey(KEY_NULL);

  currentState = MAIN_MENU;

  game = InitGame();

  initRenderer();
}

void Reload() { initRenderer(); }

int Update() {
  GameLoop(game);
  return 0;
}

void Clean() { CloseWindow(); }
