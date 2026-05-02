#include "input_handler.hpp"
#include "game.hpp"
#include "logger.hpp"
#include <raylib.h>

void InputHandler::process(Game *game) {
  if (WindowShouldClose()) {
    LOG_INFO("Close window", nullptr);
    UpdateGameState(game, GameStateType::Exit);
  }

  switch (game->state) {
  case GameStateType::MainMenu:
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
      LOG_INFO("Menu move up", nullptr);
      MenuMoveUp(game);
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
      LOG_INFO("Menu move down", nullptr);
      MenuMoveDown(game);
    }
    if (IsKeyPressed(KEY_ENTER)) {
      LOG_INFO("Menu select option", nullptr);
      MenuSelectOption(game);
    }
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

  case GameStateType::Running: {
    Player *player = game->playerManager.getPlayer(0);
    if (player->isAlive()) {
      if (IsKeyPressed(KEY_W) || IsKeyDown(KEY_W)) {
        player->move(Direction::North, game->grid);
      }
      if (IsKeyPressed(KEY_S) || IsKeyDown(KEY_S)) {
        player->move(Direction::South, game->grid);
      }
      if (IsKeyPressed(KEY_A) || IsKeyDown(KEY_A)) {
        player->move(Direction::West, game->grid);
      }
      if (IsKeyPressed(KEY_D) || IsKeyDown(KEY_D)) {
        player->move(Direction::East, game->grid);
      }
      if (IsKeyPressed(KEY_SPACE)) {
        game->bombManager.plantBomb(*player, game->grid, GetTime());
      }
      if (IsKeyPressed(KEY_P)) {
        LOG_INFO("Switch pause state", nullptr);
        PauseSwitchState(game);
      }
    }
    break;
  }

  case GameStateType::PauseMenu:
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
      LOG_INFO("Switch pause state", nullptr);
      PauseSwitchState(game);
    }
    break;

  case GameStateType::Exit:
    break;
  }
}
