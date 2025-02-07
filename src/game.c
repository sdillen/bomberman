#include "game.h"
#include "input.h"
#include "renderer.h"
#include <raylib.h>
#include <stdlib.h>
Game *game;
// Game state functions
void mainMenuState(Game *game);
void runningState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

// Grid functions
void initGrid(Cell grid[GRID_WIDTH][GRID_HEIGHT]);
_Bool isSolidWall(Position position);
_Bool isSpawn(Position position);
void updateCell(Position position, CellType cellType);

void InitGame() {
  // Initialize core game
  game = (Game *)malloc(sizeof(Game));
  game->title = "Bomberman";
  // Set init game state
  UpdateGameState(game, MAIN_MENU);
  // Initialize main menu
  MainMenu *mainMenu = (MainMenu *)malloc(sizeof(MainMenu));
  game->mainMenu = mainMenu;
  game->mainMenu->options[0] = "Neues Spiel";
  game->mainMenu->options[1] = "Beenden";
  // Initialize pause menu
  PauseMenu *pauseMenu = (PauseMenu *)malloc(sizeof(PauseMenu));
  game->pauseMenu = pauseMenu;
  game->pauseMenu->title = "Pause";
  // Initialize grid/map
  initGrid(game->grid);
  // Initialize player
  // TODO: Beide Spieler dynamisch erstellen
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = (Player *)malloc(sizeof(Player));
    player->entity.id = i;
    player->entity.position.x = 1;
    player->entity.position.y = 1;
    player->entity.targetPosition = player->entity.position;
    player->entity.facing = EAST;
    player->isAlive = 1;
    player->speed = 5;
    game->player[i] = player;
  }
}

void initGrid(Cell grid[GRID_WIDTH][GRID_HEIGHT]) {
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Position position = {x, y};
      if (isSolidWall(position)) {
        updateCell(position, CELL_SOLID_WALL);
      } else if (isSpawn(position)) {
        updateCell(position, CELL_EMPTY);
      } else {
        updateCell(position, CELL_DESTRUCTIBLE);
      }
    };
  };
}

_Bool isSolidWall(Position position) {
  if (position.x == 0) {
    return 1;
  } else if (position.x == GRID_WIDTH - 1) {
    return 1;
  }
  if (position.y == 0) {
    return 1;
  } else if (position.y == GRID_HEIGHT - 1) {
    return 1;
  }
  if (position.x % 2 == 0 && position.y % 2 == 0) {
    return 1;
  }
  return 0;
}

_Bool isSpawn(Position position) {
  if ((position.x == 1 || position.x == GRID_WIDTH - 2) &&
      (position.y <= 3 || position.y >= GRID_HEIGHT - 4)) {
    return 1;
  }
  if ((position.y == 1 || position.y == GRID_HEIGHT - 2) &&
      (position.x <= 3 || position.x >= GRID_WIDTH - 4)) {
    return 1;
  }
  return 0;
}

void updateCell(Position position, CellType cellType) {
  game->grid[position.x][position.y].type = cellType;
}

_Bool isCollision(Position next) {
  if (game->grid[next.x][next.y].type == CELL_EMPTY) {
    return 0;
  }
  return 1;
}

void MovePlayer(Player *player, Direction direction) {
  // Keinen Animationsabbruch
  if (player->state == IDLE) {
    Position targetPosition = player->entity.targetPosition;
    switch (direction) {
    case NORTH:
      targetPosition.y -= 1;
      if (!isCollision(targetPosition)) {
        player->entity.targetPosition = targetPosition;
      }
      break;
    case EAST:
      targetPosition.x += 1;
      if (!isCollision(targetPosition)) {
        player->entity.targetPosition = targetPosition;
        player->entity.facing = EAST;
      }
      break;
    case SOUTH:
      targetPosition.y += 1;
      if (!isCollision(targetPosition)) {
        player->entity.targetPosition = targetPosition;
      }
      break;
    case WEST:
      targetPosition.x -= 1;
      if (!isCollision(targetPosition)) {
        player->entity.targetPosition = targetPosition;
        player->entity.facing = WEST;
      }
      break;
    }
    player->state = WALKING;
  }
};

void UpdatePlayerPositionProgress(Player *player) {
  if (player->state == WALKING) {
    player->entity.progress += game->deltaTime * player->speed;
    if (player->entity.progress >= 1.0f) {
      player->entity.progress = 0;
      player->entity.position = player->entity.targetPosition;
      UpdatePlayerState(player, IDLE);
    }
  }
}

void UpdatePlayerState(Player *player, PlayerState state) {
  player->state = state;
}

void GameLoop() {
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
      UpdateGameState(game, RUNNING);
      break;
    case 1:
      UpdateGameState(game, EXIT);
      break;
    }
  }
}

void runningState(Game *game) {
  game->deltaTime = GetFrameTime();
  if (game->pauseMenu->isActive) {
    UpdateGameState(game, PAUSE_MENU);
  }
  UpdatePlayerPositionProgress(game->player[0]);
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
