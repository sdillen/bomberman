#include "game.h"
#include "grid.h"
#include "input.h"
#include "renderer.h"
#include <raylib.h>
#include <stdlib.h>

// Game state functions
void mainMenuState(Game *game);
void runningState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

Game *InitGame() {
  Game *game = (Game *)malloc(sizeof(Game));
  game->title = "Bomberman";
  UpdateGameState(game, MAIN_MENU);
  MainMenu *mainMenu = (MainMenu *)malloc(sizeof(MainMenu));
  game->mainMenu = mainMenu;
  game->mainMenu->options[0] = "Neues Spiel";
  game->mainMenu->options[1] = "Beenden";
  PauseMenu *pauseMenu = (PauseMenu *)malloc(sizeof(PauseMenu));
  game->pauseMenu = pauseMenu;
  game->pauseMenu->title = "Pause";
  return game;
}

void GameLoop(Game *game) {
  while (game->state != EXIT) {
    HandleInput(game);
    game->stateFunction(game);
    Render(game);
  }
};

void UpdateGameState(Game *game, GameStateType stateType) {
  // Setze den Game Zustand und setze die zugehörige Funktion
  game->state = stateType;
  switch (stateType) {
  case MAIN_MENU:
    game->stateFunction = mainMenuState;
    break;
  case RUNNING:
    game->stateFunction = runningState;
    break;
  case PAUSE_MENU:
    game->stateFunction = pauseState;
    break;
  case EXIT:
    game->stateFunction = exitState;
    break;
  }
}

void mainMenuState(Game *game) {
  if (game->mainMenu->next) {
    switch (game->mainMenu->selectedOption) {
    case 0:
      initGrid();
      UpdateGameState(game, RUNNING);
      break;
    case 1:
      UpdateGameState(game, EXIT);
      break;
    }
  }
}

void runningState(Game *game) {
  if (game->pauseMenu->isActive) {
    UpdateGameState(game, PAUSE_MENU);
  }
}

void pauseState(Game *game) {
  if (!game->pauseMenu->isActive) {
    UpdateGameState(game, RUNNING);
  }
}
void exitState(Game *game) {}

void MenuMoveUp(Game *game) {
  game->mainMenu->selectedOption =
      (game->mainMenu->selectedOption - 1 + MENU_OPTIONS) % MENU_OPTIONS;
}

void MenuMoveDown(Game *game) {
  game->mainMenu->selectedOption =
      (game->mainMenu->selectedOption + 1 + MENU_OPTIONS) % MENU_OPTIONS;
}

void MenuSelectOption(Game *game) { game->mainMenu->next = 1; }

void PauseSwitchState(Game *game) {
  game->pauseMenu->isActive = game->pauseMenu->isActive ? 0 : 1;
}
