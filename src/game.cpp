#include "game.hpp"
#include "input_handler.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
#include <random>

static std::mt19937 rng(std::random_device{}());

void mainMenuState(Game *game);
void charSelectMenuState(Game *game);
void runningCountdownState(Game *game);
void runningState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

void checkPlayerOnPowerUp(Player &player, Grid &grid, std::mt19937 &rng);
void collectPowerUp(Position pos, Player &player, Grid &grid,
                    std::mt19937 &rng);

Game *InitGame() {
  Game *game = new Game{};
  if (game == nullptr) {
    LOG_ERROR("Allocation of game failed!", nullptr);
  }
  game->title = "Bomberman";
  UpdateGameState(game, GameStateType::MainMenu);

  game->mainMenu = std::make_unique<MainMenu>();
  game->mainMenu->options[0] = "Neues Spiel";
  game->mainMenu->options[1] = "Beenden";

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
    LOG_DEBUG("game->state: mainMenuState", nullptr);
    game->stateFunction = mainMenuState;
    break;
  case GameStateType::CharSelectMenu:
    LOG_DEBUG("game->state: charSelectMenuState", nullptr);
    game->stateFunction = charSelectMenuState;
    break;
  case GameStateType::RunningCountdown:
    LOG_DEBUG("game->state: runningCountdownState", nullptr);
    game->stateFunction = runningCountdownState;
    break;
  case GameStateType::Running:
    LOG_DEBUG("game->state: runningState", nullptr);
    game->stateFunction = runningState;
    break;
  case GameStateType::PauseMenu:
    LOG_DEBUG("game->state: pauseState", nullptr);
    game->stateFunction = pauseState;
    break;
  case GameStateType::Exit:
    LOG_DEBUG("game->state: exitState", nullptr);
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
      UpdateGameState(game, GameStateType::CharSelectMenu);
      break;
    case 1:
      UpdateGameState(game, GameStateType::Exit);
      break;
    }
  }
}

void charSelectMenuState(Game *game) {
  if (game->charSelectMenu->next) {
    auto &players = game->playerManager.getPlayers();
    for (int i = 0; i < kMaxPlayers; i++) {
      if (i == 0) {
        Renderer::setCharAnimationSet(game->charSelectMenu->selectedOption,
                            players[i].get());
      } else {
        int char_id = std::uniform_int_distribution(0, kCharacters - 1)(rng);
        Renderer::setCharAnimationSet(char_id, players[i].get());
      }
    }
    UpdateGameState(game, GameStateType::RunningCountdown);
  }
}

void runningCountdownState(Game *game) {
  LOG_DEBUG("runningCountdownState", nullptr);
  game->deltaTime = GetFrameTime();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->playerManager.getPlayer(i);
    auto *anim = player->getAnimation(PlayerState::Spawn);
    if (anim && anim->isLastFrame()) {
      player->setState(PlayerState::Idle);
    }
  }
  game->countdown -= game->deltaTime;
  if (game->countdown <= 0.0f) {
    UpdateGameState(game, GameStateType::Running);
  }
}

void runningState(Game *game) {
  LOG_DEBUG("runnigState", nullptr);
  game->deltaTime = GetFrameTime();
  if (game->pauseMenu->isActive) {
    UpdateGameState(game, GameStateType::PauseMenu);
  }

  game->playerManager.updateMovement(game->deltaTime);
  game->bombManager.updateAll(game->playerManager, game->deltaTime,
                              game->grid, rng);

  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->playerManager.getPlayer(i);
    checkPlayerOnPowerUp(*player, game->grid, rng);
  }

  game->playerManager.checkAllAlive();
}

void pauseState(Game *game) {
  if (!game->pauseMenu->isActive) {
    UpdateGameState(game, GameStateType::Running);
  }
}

void exitState(Game *game) {}

void checkPlayerOnPowerUp(Player &player, Grid &grid, std::mt19937 &rng) {
  Position pos = player.getPosition();
  Cell cell = grid.getCell(pos);
  if (cell.type == CellType::PowerUp) {
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
