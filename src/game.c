#include "game.h"
#include "input.h"
#include "log.h"
#include "renderer.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
Game *game;
// Game state functions
void mainMenuState(Game *game);
void runningCountdownState(Game *game);
void runningState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

// Grid functions
void initGrid(Cell grid[GRID_WIDTH][GRID_HEIGHT]);
_Bool isSolidWall(Position position);
_Bool isSpawnProtected(Position position);
Position getSpawn(int id);
void updateCell(Position position, CellType cellType);
_Bool isCollision(Position next);

// Player functions
Player *initPlayer(int id);

// Bomb
void createExplosion(Bomb *bomb, int blastRadius);

// Destructible
void breakDestructibel(Position pos);

// Items
void checkPlayerOnPowerUp(Player *player);
void collectPowerUp(Position pos, Player *player);

Game *InitGame() {
  // Initialize core game
  game = (Game *)malloc(sizeof(Game));
  if (game == NULL) {
    LOG_ERROR("Allocation of game failed!", NULL);
  }
  game->title = "Bomberman";
  // Set init game state
  UpdateGameState(game, MAIN_MENU);
  // Initialize main menu
  MainMenu *mainMenu = (MainMenu *)malloc(sizeof(MainMenu));
  if (mainMenu == NULL) {
    LOG_ERROR("Allocation of main menu failed!", NULL);
  }
  game->mainMenu = mainMenu;
  game->mainMenu->options[0] = "Neues Spiel";
  game->mainMenu->options[1] = "Beenden";
  // Initialize pause menu
  PauseMenu *pauseMenu = (PauseMenu *)malloc(sizeof(PauseMenu));
  if (pauseMenu == NULL) {
    LOG_ERROR("Allocation of pause menu failed!", NULL);
  }
  game->pauseMenu = pauseMenu;
  game->pauseMenu->title = "Pause";
  // Countdown
  game->countdown = 3;
  // Initialize grid
  initGrid(game->grid);
  // Initialize players
  for (int i = 0; i < MAX_PLAYERS; i++) {
    game->player[i] = initPlayer(i);
  }
  return game;
}

void initGrid(Cell grid[GRID_WIDTH][GRID_HEIGHT]) {
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Position position = {x, y};
      if (isSolidWall(position)) {
        updateCell(position, CELL_SOLID_WALL);
      } else if (isSpawnProtected(position)) {
        updateCell(position, CELL_EMPTY);
      } else {
        // 80% Chance to create a destructible
        if (rand() % 10 <= 7) {
          updateCell(position, CELL_DESTRUCTIBLE);
        }
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

_Bool isSpawnProtected(Position position) {
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

Position getSpawn(int id) {
  // 4 possible spawn locations
  // this will support MAX_PLAYERS = 4
  switch (id) {
  case 0:
    return (Position){1, 1};
  case 1:
    return (Position){13, 13};
  case 2:
    return (Position){13, 1};
  case 3:
    return (Position){1, 13};
  }
  return (Position){0, 0};
}

void updateCell(Position position, CellType cellType) {
  game->grid[position.x][position.y].type = cellType;
}

_Bool isCollision(Position next) {
  if (game->grid[next.x][next.y].type == CELL_EMPTY) {
    return 0;
  }
  if (game->grid[next.x][next.y].type == CELL_POWERUP) {
    return 0;
  }
  return 1;
}

Player *initPlayer(int id) {
  Player *player = (Player *)malloc(sizeof(Player));
  if (player == NULL) {
    LOG_ERROR("Allocation of player failed!", NULL);
  }
  player->entity.id = id;
  player->entity.position = getSpawn(id);
  player->entity.targetPosition = player->entity.position;
  switch (id) {
  case (0):
  case (3):
    player->entity.facing = EAST;
    break;
  case (1):
  case (2):
    player->entity.facing = WEST;
    break;
  }
  player->isAlive = 1;
  player->speed = 5;
  player->blastRadius = 3;
  player->bombs = 1;
  for (int i = 0; i < player->bombs; i++) {
    player->bombList[i] = NULL;
  }
  return player;
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
    default:
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

void PlantBomb(Player *player) {
  for (int i = 0; i < player->bombs; i++) {
    if (player->bombList[i] == NULL) {
      LOG_INFO("Bomb planted", NULL);
      player->bombList[i] = (Bomb *)malloc(sizeof(Bomb));
      player->bombList[i]->entity.position = player->entity.position;
      player->bombList[i]->startTime = GetTime();
      player->bombList[i]->endTime = player->bombList[i]->startTime + 3;
      player->bombList[i]->animation =
          CreateAnimation(bombSparkFrames, BOMB_SPARK_FRAMES_NUM, 0.1f);
      for (int j = 0; j < _DIRECTION_NUM; j++) {
        player->bombList[i]->explosion[j] = NULL;
      }
      break;
    }
  }
};

void UpdateBombTimer(Player *player) {
  for (int i = 0; i < player->bombs; i++) {
    if (player->bombList[i] != NULL) {
      Bomb *bomb = player->bombList[i];
      if (bomb->endTime <= GetTime() && bomb->endTime != 0) {
        createExplosion(player->bombList[i], player->blastRadius);
        bomb->startTime = 0;
        bomb->endTime = 0;
      }
    }
  }
}

void RemoveExplodedBombs(Player *player) {
  for (int i = 0; i < player->bombs; i++) {
    if (player->bombList[i] != NULL) {
      if (player->bombList[i]->isExploded) {
        free(player->bombList[i]);
        player->bombList[i] = NULL;
      }
    }
  }
}

void createExplosion(Bomb *bomb, int blastRadius) {
  double startTime = GetTime();
  Position pos = bomb->entity.position;
  for (int i = 0; i < _DIRECTION_NUM; i++) {
    Explosion *explosion = (Explosion *)malloc(sizeof(Explosion));
    bomb->explosion[i] = explosion;
    explosion->speed = 6;
    explosion->entity.position = pos;
    explosion->startTime = startTime;
    explosion->animation =
        CreateAnimation(explosionBlastFrames, EXPLOSION_BLAST_FRAMES_NUM, 0.1f);
    switch (i) {
    case NORTH:
      explosion->entity.targetPosition = (Position){pos.x, pos.y - blastRadius};
      break;
    case EAST:
      explosion->entity.targetPosition = (Position){pos.x + blastRadius, pos.y};
      break;
    case SOUTH:
      explosion->entity.targetPosition = (Position){pos.x, pos.y + blastRadius};
      break;
    case WEST:
      explosion->entity.targetPosition = (Position){pos.x - blastRadius, pos.y};
      break;
    }
  }
}

void updateExplosionProgress(Player *player) {
  for (int i = 0; i < player->bombs; i++) {
    if (player->bombList[i] != NULL) {
      Bomb *bomb = player->bombList[i];
      if (bomb->endTime == 0) {
        int nullExplosions = 0;
        for (int j = 0; j < _DIRECTION_NUM; j++) {
          if (bomb->explosion[j] != NULL) {
            Explosion *explosion = bomb->explosion[j];
            float progress = game->deltaTime * explosion->speed;
            float relPos =
                explosion->entity.progress * (player->blastRadius + 1);
            for (int k = 0; k <= player->blastRadius; k++) {
              if (relPos >= k) {
                Position cellPos;
                switch (j) {
                case NORTH:
                  cellPos = (Position){explosion->entity.position.x,
                                       explosion->entity.position.y - k};
                  break;
                case EAST:
                  cellPos = (Position){explosion->entity.position.x + k,
                                       explosion->entity.position.y};
                  break;
                case SOUTH:
                  cellPos = (Position){explosion->entity.position.x,
                                       explosion->entity.position.y + k};
                  break;
                case WEST:
                  cellPos = (Position){explosion->entity.position.x - k,
                                       explosion->entity.position.y};
                  break;
                }
                Cell cell = game->grid[cellPos.x][cellPos.y];
                if (cell.type == CELL_DESTRUCTIBLE) {
                  breakDestructibel(cellPos);
                  bomb->explosion[j] = NULL;
                  break;
                  //  explosion->entity.progress = 1;
                }
                if (cell.type == CELL_SOLID_WALL) {
                  bomb->explosion[j] = NULL;
                  break;
                  //  explosion->entity.progress = 1;
                }
                for (int l = 0; l < MAX_PLAYERS; l++) {
                  Player *player = game->player[l];
                  if (player->entity.position.x == cellPos.x &&
                      player->entity.position.y == cellPos.y) {
                    bomb->explosion[j] = NULL;
                    player->isAlive = 0;
                    break;
                  }
                }
              }
            }
            explosion->entity.progress += progress;
            if (explosion->entity.progress >= 1) {
              bomb->explosion[j] = NULL;
            }
          } else {
            nullExplosions++;
            if (nullExplosions == _DIRECTION_NUM) {
              bomb->isExploded = 1;
            }
          }
        }
      }
    }
  }
}

void checkPlayerAlive(Player *player) {
  if (player->isAlive == 0) {
    player->state = DEAD;
  }
}

void breakDestructibel(Position pos) {
  if ((rand() % 10) <= 1) {
    updateCell(pos, CELL_POWERUP);
  } else {
    updateCell(pos, CELL_EMPTY);
  }
}

void checkPlayerOnPowerUp(Player *player) {
  Position pos = player->entity.position;
  Cell cell = game->grid[pos.x][pos.y];
  if (cell.type == CELL_POWERUP) {
    collectPowerUp(pos, player);
  }
}

void collectPowerUp(Position pos, Player *player) {
  int r = rand() % _POWERUP_NUM;
  switch (r) {
  case POWERUP_SPEED:
    printf("speed collected\n");
    player->speed++;
    break;
  case POWERUP_BOMB:
    printf("bomb collected\n");
    player->bombs++;
    break;
  case POWERUP_BLAST_RADIUS:
    printf("blast radius collected\n");
    player->blastRadius++;
    break;
  }
  updateCell(pos, CELL_EMPTY);
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
    LOG_DEBUG("game->state: mainMenuState", NULL);
    game->stateFunction = mainMenuState;
    break;
  case RUNNING_COUNTDOWN:
    LOG_DEBUG("game->state: runningCountdownState", NULL);
    game->stateFunction = runningCountdownState;
    break;
  case RUNNING:
    LOG_DEBUG("game->state: runningState", NULL);
    game->stateFunction = runningState;
    break;
  case PAUSE_MENU:
    LOG_DEBUG("game->state: pauseState", NULL);
    game->stateFunction = pauseState;
    break;
  case EXIT:
    LOG_DEBUG("game->state: exitState", NULL);
    game->stateFunction = exitState;
    break;
  }
}

void mainMenuState(Game *game) {
  if (game->mainMenu->next) {
    switch (game->mainMenu->selectedOption) {
    case 0:
      UpdateGameState(game, RUNNING_COUNTDOWN);
      break;
    case 1:
      UpdateGameState(game, EXIT);
      break;
    }
  }
}

void runningCountdownState(Game *game) {
  LOG_DEBUG("runningCountdownState", NULL);
  game->deltaTime = GetFrameTime();
  game->countdown -= game->deltaTime;
  if (game->countdown <= 0.0f) {
    UpdateGameState(game, RUNNING);
  }
}

void runningState(Game *game) {
  LOG_DEBUG("runnigState", NULL);
  game->deltaTime = GetFrameTime();
  if (game->pauseMenu->isActive) {
    UpdateGameState(game, PAUSE_MENU);
  }
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = game->player[i];
    LOG_DEBUG("%i runningState: UpdatePlayerPositionProgress", i);
    UpdatePlayerPositionProgress(player);
    LOG_DEBUG("%i runningState: UpdateBombTimer", i);
    UpdateBombTimer(player);
    LOG_DEBUG("%i runningState: updateExplosionProgress", i);
    updateExplosionProgress(player);
    LOG_DEBUG("%i runningState: RemoveExplodedBombs", i);
    RemoveExplodedBombs(player);
    LOG_DEBUG("%i runningState: checkPlayerOnPowerUp", i);
    checkPlayerOnPowerUp(player);
    LOG_DEBUG("%i runningState: checkPlayerAlive", i);
    checkPlayerAlive(player);
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
