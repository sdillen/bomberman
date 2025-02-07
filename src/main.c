#include "game.h"
#include "renderer.h"
#include <raylib.h>

// Fenster Einstellungen
static const int windowWidth = 800;
static const int windowHeight = 600;
static const char *windowTitle = "Bomberman";
static const int windowFPS = 60;

int main() {
  // Fenster erstellen und Einstellungen setzen
  InitWindow(windowWidth, windowHeight, windowTitle);
  SetTargetFPS(windowFPS);

  // Entferne Tastenbelegung vom ESC-Key zum schließen des Spiels
  SetExitKey(KEY_NULL);

  // Init
  InitGame();
  InitRenderer();

  GameLoop();

  CloseWindow();
  return 0;
}
