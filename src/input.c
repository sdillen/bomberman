#include "input.h"
#include "game.h"
#include "log.h"
#include <raylib.h>
#include <string.h>

void HandleInput(Game *game) {
  if (WindowShouldClose()) {
    LOG_INFO("Close window", NULL);
    UpdateGameState(game, EXIT);
  }
  switch (game->state) {
  case MAIN_MENU:
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
      LOG_INFO("Menu move up", NULL);
      MenuMoveUp(game);
    };
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
      LOG_INFO("Menu move down", NULL);
      MenuMoveDown(game);
    };
    if (IsKeyPressed(KEY_ENTER)) {
      LOG_INFO("Menu select option", NULL);
      MenuSelectOption(game);
    };
    break;
  case RUNNING:
    if (IsKeyPressed(KEY_W)) {
      // Move player up
    };
    if (IsKeyPressed(KEY_S)) {
      // Move player down
    };
    if (IsKeyPressed(KEY_A)) {
      // Move player left
    };
    if (IsKeyPressed(KEY_D)) {
      // Move player right
    };
    if (IsKeyPressed(KEY_SPACE)) {
      // Plant Bomb
    };
    if (IsKeyPressed(KEY_P)) {
      LOG_INFO("Switch pause state", NULL);
      PauseSwitchState(game);
    };
    break;
  case PAUSE_MENU:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
      LOG_INFO("Switch pause state", NULL);
      PauseSwitchState(game);
    };
    break;
  case EXIT:
    break;
  }
}
