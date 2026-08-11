#include "game.hpp"
#include "bot_controller.hpp"
#include "input_handler.hpp"
#include "logger.hpp"
#include "network_manager.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
#include <random>

static std::mt19937 rng(std::random_device{}());

void mainMenuState(Game *game);
void hostGameMenuState(Game *game);
void joinGameMenuState(Game *game);
void charSelectMenuState(Game *game);
void clientCharSelectState(Game *game);
void runningCountdownState(Game *game);
void runningState(Game *game);
void clientRunningState(Game *game);
void gameOverState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

void checkPlayerOnPowerUp(Player &player, Grid &grid, std::mt19937 &rng);
void collectPowerUp(Position pos, Player &player, Grid &grid,
                    std::mt19937 &rng);

Game *InitGame() {
  Game *game = new Game{};
  if (game == nullptr) {
    LOG_ERROR("Allocation of game failed!");
  }
  game->title = "Bomberman";
  UpdateGameState(game, GameStateType::MainMenu);

  game->mainMenu = std::make_unique<MainMenu>();
  game->mainMenu->options[0] = "Lokal Spielen";
  game->mainMenu->options[1] = "LAN Host";
  game->mainMenu->options[2] = "LAN Beitreten";
  game->mainMenu->options[3] = "Beenden";

  game->pauseMenu = std::make_unique<PauseMenu>();
  game->pauseMenu->title = "Pause";

  game->charSelectMenu = std::make_unique<CharSelectMenu>();
  game->charSelectMenu->title = "Wähle deinen Character";

  game->countdown = 3;
  game->grid.init(rng);
  game->playerManager.initAll();
  return game;
}

void GameLoop(Game *game) {
  while (game->state != GameStateType::Exit) {
    InputHandler::process(game);
    game->stateFunction(game);
    Renderer::render(game);
  }
}

void UpdateGameState(Game *game, GameStateType stateType) {
  game->state = stateType;
  switch (stateType) {
  case GameStateType::MainMenu:
    LOG_DEBUG("game->state: mainMenuState");
    game->stateFunction = mainMenuState;
    break;
  case GameStateType::HostGameMenu:
    LOG_DEBUG("game->state: hostGameMenuState");
    game->stateFunction = hostGameMenuState;
    break;
  case GameStateType::JoinGameMenu:
    LOG_DEBUG("game->state: joinGameMenuState");
    game->stateFunction = joinGameMenuState;
    break;
  case GameStateType::CharSelectMenu:
    LOG_DEBUG("game->state: charSelectMenuState");
    game->stateFunction = charSelectMenuState;
    break;
  case GameStateType::ClientCharSelect:
    LOG_DEBUG("game->state: clientCharSelectState");
    game->stateFunction = clientCharSelectState;
    break;
  case GameStateType::RunningCountdown:
    LOG_DEBUG("game->state: runningCountdownState");
    game->stateFunction = runningCountdownState;
    break;
  case GameStateType::Running:
    LOG_DEBUG("game->state: runningState");
    game->stateFunction = runningState;
    break;
  case GameStateType::ClientRunning:
    LOG_DEBUG("game->state: clientRunningState");
    game->stateFunction = clientRunningState;
    break;
  case GameStateType::GameOver:
    LOG_DEBUG("game->state: gameOverState");
    game->stateFunction = gameOverState;
    break;
  case GameStateType::PauseMenu:
    LOG_DEBUG("game->state: pauseState");
    game->stateFunction = pauseState;
    break;
  case GameStateType::Exit:
    LOG_DEBUG("game->state: exitState");
    game->stateFunction = exitState;
    break;
  }
}

void DestroyGame(Game *game) { delete game; }

void MenuMoveUp(Game *game) {
  game->mainMenu->selectedOption =
      (game->mainMenu->selectedOption - 1 + kMenuOptions) % kMenuOptions;
}

void MenuMoveDown(Game *game) {
  game->mainMenu->selectedOption =
      (game->mainMenu->selectedOption + 1 + kMenuOptions) % kMenuOptions;
}

void MenuSelectOption(Game *game) { game->mainMenu->next = true; }

void NextChar(Game *game) {
  game->charSelectMenu->selectedOption =
      (game->charSelectMenu->selectedOption + 1 + kCharacters) % kCharacters;
}

void PrevChar(Game *game) {
  game->charSelectMenu->selectedOption =
      (game->charSelectMenu->selectedOption - 1 + kCharacters) % kCharacters;
}

void SelectChar(Game *game) { game->charSelectMenu->next = true; }

void PauseSwitchState(Game *game) {
  game->pauseMenu->isActive = !game->pauseMenu->isActive;
}

void mainMenuState(Game *game) {
  if (game->mainMenu->next) {
    switch (game->mainMenu->selectedOption) {
    case 0:
      game->isServer = false;
      game->isClient = false;
      game->numHumanPlayers = 1;
      game->mainMenu->next = false;
      UpdateGameState(game, GameStateType::CharSelectMenu);
      break;
    case 1:
      game->isClient = false;
      game->mainMenu->next = false;
      UpdateGameState(game, GameStateType::HostGameMenu);
      break;
    case 2:
      game->isServer = false;
      game->isClient = true;
      game->mainMenu->next = false;
      UpdateGameState(game, GameStateType::JoinGameMenu);
      break;
    case 3:
      UpdateGameState(game, GameStateType::Exit);
      break;
    }
  }
}

void charSelectMenuState(Game *game) {
  auto &players = game->playerManager.getPlayers();

  if (game->charSelectMenu->next && game->currentSelectingPlayer == 0) {
    // Host has confirmed their selection
    game->charSelections[0] = game->charSelectMenu->selectedOption;
    Renderer::setCharAnimationSet(game->charSelections[0], players[0].get());

    // Assign random chars to bots
    for (int i = game->numHumanPlayers; i < kMaxPlayers; i++) {
      int char_id = std::uniform_int_distribution(0, kCharacters - 1)(rng);
      game->charSelections[i] = char_id;
      Renderer::setCharAnimationSet(char_id, players[i].get());
    }

    if (game->isServer && game->numHumanPlayers > 1) {
      game->currentSelectingPlayer = 1;
      game->charSelectMenu->next = false;
      LOG_INFO("Host: sending char select turn to client, numHumans=%d",
               game->numHumanPlayers);
      NetworkManager::sendCharSelect(
          game->currentSelectingPlayer, 0,
          game->currentSelectingPlayer - 1);
    } else {
      UpdateGameState(game, GameStateType::RunningCountdown);
    }
    return;
  }

  // Host waiting for client selections
  if (game->isServer && game->currentSelectingPlayer > 0) {
    uint8_t fromPlayer, charIndex;
    if (NetworkManager::receiveCharSelect(fromPlayer, charIndex)) {
      LOG_INFO("Host: received char select from player %d (char %d)",
               fromPlayer, charIndex);
      game->charSelections[fromPlayer] = charIndex;
      Renderer::setCharAnimationSet(charIndex, players[fromPlayer].get());
      game->currentSelectingPlayer++;

      if (game->currentSelectingPlayer < game->numHumanPlayers) {
        LOG_INFO("Host: sending turn to next player %d",
                 game->currentSelectingPlayer);
        NetworkManager::sendCharSelect(
            game->currentSelectingPlayer, 0,
            game->currentSelectingPlayer - 1);
      } else {
        // All human players have selected
        uint8_t charIds[4];
        for (int i = 0; i < 4; i++)
          charIds[i] = static_cast<uint8_t>(game->charSelections[i]);
        for (int i = 1; i < game->numHumanPlayers; i++)
          NetworkManager::sendAssignment(i, game->numHumanPlayers, charIds,
                                          i - 1);
        game->currentSelectingPlayer = 0;
        UpdateGameState(game, GameStateType::RunningCountdown);
      }
    }
  }
}

void runningCountdownState(Game *game) {
  LOG_DEBUG("runningCountdownState");
  game->deltaTime = GetFrameTime();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->playerManager.getPlayer(i);
    auto *anim = player->getAnimation(PlayerState::Spawn);
    if (anim && anim->isLastFrame()) {
      player->setState(PlayerState::Idle);
    }
  }
  game->countdown -= game->deltaTime;
  if (game->isServer) {
    NetworkManager::broadcastGameState(game);
  }
  if (game->countdown <= 0.0f) {
    UpdateGameState(game, GameStateType::Running);
  }
}

void runningState(Game *game) {
  LOG_DEBUG("runnigState");
  game->deltaTime = GetFrameTime();
  if (game->pauseMenu->isActive) {
    UpdateGameState(game, GameStateType::PauseMenu);
  }

  if (game->isServer) {
    int playerId, direction;
    uint8_t plantBomb;
    while (NetworkManager::receivePlayerInput(playerId, direction, plantBomb)) {
      Player *player = game->playerManager.getPlayer(playerId);
      if (player && player->isAlive() && direction >= 0) {
        player->move(static_cast<Direction>(direction), game->grid);
      }
      if (player && player->isAlive() && plantBomb) {
        game->bombManager.plantBomb(*player, game->grid, GetTime());
      }
    }
  }

  BotController::updateAll(game->playerManager, game->bombManager,
                           game->grid, game->deltaTime, GetTime(),
                           game->numHumanPlayers, rng);
  game->playerManager.updateMovement(game->deltaTime);
  game->bombManager.updateAll(game->playerManager, game->deltaTime,
                              game->grid, rng);

  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->playerManager.getPlayer(i);
    checkPlayerOnPowerUp(*player, game->grid, rng);
  }

  game->playerManager.checkAllAlive();

  int aliveCount = 0;
  int lastAliveId = -1;
  for (int i = 0; i < kMaxPlayers; i++) {
    if (game->playerManager.getPlayer(i)->isAlive()) {
      aliveCount++;
      lastAliveId = i;
    }
  }
  if (aliveCount <= 1 && game->state == GameStateType::Running) {
    game->winnerId = lastAliveId;
    game->gameOverTimer = 5.0f;
    if (game->isServer) {
      NetworkManager::broadcastGameState(game);
    }
    UpdateGameState(game, GameStateType::GameOver);
    return;
  }

  if (game->isServer) {
    NetworkManager::broadcastGameState(game);
  }
}

void pauseState(Game *game) {
  if (!game->pauseMenu->isActive) {
    UpdateGameState(game, GameStateType::Running);
  }
}

void exitState(Game *game) {}

void hostGameMenuState(Game *game) {
  if (!game->isServer) {
    game->isServer = true;
    NetworkManager::startServer();
    game->numConnectedClients = 0;
    game->hostPlayerCount = 2;
    game->mainMenu->next = false;
  }
  NetworkManager::pollEvents();
  game->numConnectedClients = NetworkManager::getNumConnectedPeers();

  if (game->mainMenu->next) {
    game->numHumanPlayers = game->hostPlayerCount;
    game->currentSelectingPlayer = 0;
    game->charSelectMenu->next = false;
    UpdateGameState(game, GameStateType::CharSelectMenu);
  }
}

void joinGameMenuState(Game *game) {
  if (game->isConnecting) {
    NetworkManager::pollEvents();
    if (NetworkManager::isConnected()) {
      game->connectionFailed = false;
      game->isConnecting = false;
      LOG_INFO("Connected to server");
    }
  }

  if (NetworkManager::isConnected()) {
    game->isConnecting = false;
    game->connectionFailed = false;
    LOG_INFO("Client: connected, going to char select");
    UpdateGameState(game, GameStateType::ClientCharSelect);
  }
}

void gameOverState(Game *game) {
  game->deltaTime = GetFrameTime();
  if (game->isServer) {
    NetworkManager::pollEvents();
    NetworkManager::broadcastGameState(game);
  } else if (game->isClient) {
    NetworkManager::receiveGameState(game);
  }
  game->gameOverTimer -= game->deltaTime;

  if (game->mainMenu->next || game->gameOverTimer <= 0.0f) {
    game->mainMenu->next = false;
    game->winnerId = -1;
    NetworkManager::disconnect();
    game->isServer = false;
    game->isClient = false;
    game->currentSelectingPlayer = 0;
    game->charSelectMenu->next = false;
    game->countdown = 3;
    game->bombManager.clearAll();
    game->grid.init(rng);
    game->playerManager.initAll();
    UpdateGameState(game, GameStateType::MainMenu);
  }
}

void clientCharSelectState(Game *game) {
  if (!NetworkManager::isConnected()) {
    LOG_INFO("Client: disconnected in char select");
    game->winnerId = -2;
    game->gameOverTimer = 5.0f;
    UpdateGameState(game, GameStateType::GameOver);
    return;
  }

  if (game->currentSelectingPlayer == 0) {
    uint8_t fromPlayer, charIndex;
    if (NetworkManager::receiveCharSelect(fromPlayer, charIndex)) {
      game->currentSelectingPlayer = fromPlayer;
      game->clientPlayerId = fromPlayer;
      LOG_INFO("Client: received turn, now selecting (player %d)", fromPlayer);
    }
  } else {
    LOG_DEBUG("Client: waiting for input, curSelP=%d next=%d",
              game->currentSelectingPlayer, game->charSelectMenu->next);
  }

  if (game->charSelectMenu->next && game->currentSelectingPlayer > 0) {
    NetworkManager::sendCharSelect(game->clientPlayerId,
                                    game->charSelectMenu->selectedOption);
    LOG_INFO("Client: sent char choice %d for player %d",
             game->charSelectMenu->selectedOption, game->clientPlayerId);
    game->charSelectMenu->next = false;
    game->currentSelectingPlayer = -1;
  }

  if (game->currentSelectingPlayer == -1) {
    uint8_t assignedId, numPlayers;
    uint8_t charIds[4]{};
    if (NetworkManager::receiveAssignment(assignedId, numPlayers, charIds)) {
      game->clientPlayerId = assignedId;
      game->numHumanPlayers = numPlayers;
      for (int i = 0; i < kMaxPlayers; i++) {
        game->charSelections[i] = charIds[i];
      }
      auto &players = game->playerManager.getPlayers();
      for (int i = 0; i < kMaxPlayers; i++) {
        Renderer::setCharAnimationSet(charIds[i], players[i].get());
      }
      LOG_INFO("Client: assigned as player %d, starting game", assignedId);
      game->currentSelectingPlayer = 0;
      game->clientCountdown = 3.0f;
      UpdateGameState(game, GameStateType::ClientRunning);
    }
  }
}

void clientRunningState(Game *game) {
  game->deltaTime = GetFrameTime();
  if (!NetworkManager::isConnected()) {
    game->winnerId = -2;
    game->gameOverTimer = 5.0f;
    UpdateGameState(game, GameStateType::GameOver);
    return;
  }
  if (game->clientCountdown > 0) {
    game->clientCountdown -= game->deltaTime;
  }
  NetworkManager::receiveGameState(game);
  game->bombManager.updateExplosions(game->deltaTime);

  int aliveCount = 0;
  int lastAliveId = -1;
  for (int i = 0; i < kMaxPlayers; i++) {
    if (game->playerManager.getPlayer(i)->isAlive()) {
      aliveCount++;
      lastAliveId = i;
    }
  }
  if (aliveCount <= 1 && game->state == GameStateType::ClientRunning) {
    game->winnerId = lastAliveId;
    game->gameOverTimer = 5.0f;
    UpdateGameState(game, GameStateType::GameOver);
  }
}

void checkPlayerOnPowerUp(Player &player, Grid &grid, std::mt19937 &rng) {
  Position pos = player.getPosition();
  if (grid.getCell(pos) == CellType::PowerUp) {
    collectPowerUp(pos, player, grid, rng);
  }
}

void collectPowerUp(Position pos, Player &player, Grid &grid,
                    std::mt19937 &rng) {
  int r = std::uniform_int_distribution(0, kPowerUpCount - 1)(rng);
  switch (static_cast<PowerUpType>(r)) {
  case PowerUpType::Speed:
    printf("speed collected\n");
    player.addSpeed();
    break;
  case PowerUpType::Bomb:
    printf("bomb collected\n");
    player.addBombCount();
    break;
  case PowerUpType::BlastRadius:
    printf("blast radius collected\n");
    player.addBlastRadius();
    break;
  }
  grid.setCell(pos, CellType::Empty);
}
