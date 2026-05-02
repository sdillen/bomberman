#include "input.hpp"
#include "game.hpp"
#include "log.hpp"
#include <raylib.h>
#include <cstring>

void HandleInput(Game *game) {
  if (WindowShouldClose()) {
    LOG_INFO("Close window", nullptr);
    UpdateGameState(game, GameStateType::Exit);
  }
  Player *player = game->player[0].get();
  switch (game->state) {
  case GameStateType::MainMenu:
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
      LOG_INFO("Menu move up", nullptr);
      MenuMoveUp(game);
    };
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
      LOG_INFO("Menu move down", nullptr);
      MenuMoveDown(game);
    };
    if (IsKeyPressed(KEY_ENTER)) {
      LOG_INFO("Menu select option", nullptr);
      MenuSelectOption(game);
    };
    break;
  case GameStateType::CharSelectMenu:
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
      PrevChar(game);
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
      NextChar(game);
    }
    if (IsKeyPressed(KEY_ENTER)) {
      SelectChar(game);
    }
    break;
  case GameStateType::RunningCountdown:
    break;
  case GameStateType::Running:
    if (player->isAlive) {
      if (IsKeyPressed(KEY_W) || IsKeyDown(KEY_W)) {
        MovePlayer(player, Direction::North, game);
      };
      if (IsKeyPressed(KEY_S) || IsKeyDown(KEY_S)) {
        MovePlayer(player, Direction::South, game);
      };
      if (IsKeyPressed(KEY_A) || IsKeyDown(KEY_A)) {
        MovePlayer(player, Direction::West, game);
      };
      if (IsKeyPressed(KEY_D) || IsKeyDown(KEY_D)) {
        MovePlayer(player, Direction::East, game);
      };
      if (IsKeyPressed(KEY_SPACE)) {
        PlantBomb(player, game);
      };
      if (IsKeyPressed(KEY_P)) {
        LOG_INFO("Switch pause state", nullptr);
        PauseSwitchState(game);
      };
    }
    break;
  case GameStateType::PauseMenu:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
      LOG_INFO("Switch pause state", nullptr);
      PauseSwitchState(game);
    };
    break;
  case GameStateType::Exit:
    break;
  }
}
