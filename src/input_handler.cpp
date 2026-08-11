#include "input_handler.hpp"
#include "game.hpp"
#include "network_manager.hpp"
#include "logger.hpp"
#include <raylib.h>
#include <cstring>

void InputHandler::process(Game *game) {
  if (WindowShouldClose()) {
    LOG_INFO("Close window");
    UpdateGameState(game, GameStateType::Exit);
  }

  switch (game->state) {
  case GameStateType::MainMenu:
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
      LOG_INFO("Menu move up");
      game->mainMenu->selectedOption =
          (game->mainMenu->selectedOption - 1 + kMenuOptions) % kMenuOptions;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
      LOG_INFO("Menu move down");
      game->mainMenu->selectedOption =
          (game->mainMenu->selectedOption + 1 + kMenuOptions) % kMenuOptions;
    }
    if (IsKeyPressed(KEY_ENTER)) {
      LOG_INFO("Menu select option");
      game->mainMenu->next = true;
    }
    break;

  case GameStateType::HostGameMenu: {
    if (IsKeyPressed(KEY_LEFT)) {
      if (game->hostPlayerCount > 2) game->hostPlayerCount--;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      if (game->hostPlayerCount < 4) game->hostPlayerCount++;
    }
    if (IsKeyPressed(KEY_ENTER)) {
      if (game->numConnectedClients >= game->hostPlayerCount - 1) {
        game->mainMenu->next = true;
      }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      NetworkManager::disconnect();
      game->isServer = false;
      UpdateGameState(game, GameStateType::MainMenu);
    }
    break;
  }

  case GameStateType::JoinGameMenu: {
    if (!game->isConnecting) {
      for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
        if (IsKeyPressed(k)) {
          size_t len = strlen(game->serverIP);
          if (len < sizeof(game->serverIP) - 1) {
            game->serverIP[len] = '0' + (k - KEY_ZERO);
            game->serverIP[len + 1] = '\0';
          }
        }
      }
      if (IsKeyPressed(KEY_PERIOD)) {
        size_t len = strlen(game->serverIP);
        if (len < sizeof(game->serverIP) - 1) {
          game->serverIP[len] = '.';
          game->serverIP[len + 1] = '\0';
        }
      }
      if (IsKeyPressed(KEY_BACKSPACE)) {
        size_t len = strlen(game->serverIP);
        if (len > 0) {
          game->serverIP[len - 1] = '\0';
        }
      }
      if (IsKeyPressed(KEY_ENTER) && strlen(game->serverIP) > 0) {
        if (NetworkManager::connectToServer(game->serverIP)) {
          game->isConnecting = true;
          game->connectionFailed = false;
        } else {
          game->connectionFailed = true;
        }
      }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      NetworkManager::disconnect();
      game->isServer = false;
      game->isClient = false;
      game->isConnecting = false;
      game->connectionFailed = false;
      UpdateGameState(game, GameStateType::MainMenu);
    }
    break;
  }

  case GameStateType::CharSelectMenu:
    if (game->currentSelectingPlayer == 0) {
      if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        game->charSelectMenu->selectedOption =
            (game->charSelectMenu->selectedOption - 1 + kCharacters) %
            kCharacters;
      }
      if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        game->charSelectMenu->selectedOption =
            (game->charSelectMenu->selectedOption + 1 + kCharacters) %
            kCharacters;
      }
      if (IsKeyPressed(KEY_ENTER)) {
        game->charSelectMenu->next = true;
      }
    }
    break;

  case GameStateType::ClientCharSelect:
    if (game->currentSelectingPlayer > 0) {
      if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        game->charSelectMenu->selectedOption =
            (game->charSelectMenu->selectedOption - 1 + kCharacters) %
            kCharacters;
      }
      if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        game->charSelectMenu->selectedOption =
            (game->charSelectMenu->selectedOption + 1 + kCharacters) %
            kCharacters;
      }
      if (IsKeyPressed(KEY_ENTER)) {
        game->charSelectMenu->next = true;
      }
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
        LOG_INFO("Switch pause state");
        game->pauseMenu->isActive = !game->pauseMenu->isActive;
      }
    }
    break;
  }

  case GameStateType::GameOver:
    if (IsKeyPressed(KEY_ENTER)) {
      game->mainMenu->next = true;
    }
    break;

  case GameStateType::PauseMenu:

  case GameStateType::ClientRunning: {
    int8_t dir = -1;
    if (IsKeyPressed(KEY_W) || IsKeyDown(KEY_W)) dir = 0;
    else if (IsKeyPressed(KEY_S) || IsKeyDown(KEY_S)) dir = 2;
    else if (IsKeyPressed(KEY_A) || IsKeyDown(KEY_A)) dir = 3;
    else if (IsKeyPressed(KEY_D) || IsKeyDown(KEY_D)) dir = 1;
    uint8_t bomb = (IsKeyPressed(KEY_SPACE) || IsKeyDown(KEY_SPACE)) ? 1 : 0;
    NetworkManager::sendPlayerInput(dir, bomb);
    break;
  }

  case GameStateType::Exit:
    break;
  }
}
