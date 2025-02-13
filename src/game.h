#ifndef STATE_H
#define STATE_H
#include "util.h"
#include <raylib.h>

typedef enum {
  MAIN_MENU,
  RUNNING,
  PAUSE_MENU,
  EXIT,
} GameStateType;

typedef struct Game Game;

#define MENU_OPTIONS 2

typedef struct {
  const char *options[MENU_OPTIONS];
  int selectedOption;
  _Bool next;
} MainMenu;

#define PAUSE_OPTIONS 1

typedef struct {
  const char *title;
  _Bool isActive;
} PauseMenu;

#define GRID_WIDTH 15
#define GRID_HEIGHT 15

typedef enum {
  CELL_EMPTY,
  CELL_SOLID_WALL,
  CELL_DESTRUCTIBLE,
  CELL_POWERUP,
} CellType;

typedef struct {
  CellType type;
} Cell;

typedef enum {
  POWERUP_SPEED,
  POWERUP_BOMB,
  POWERUP_BLAST_RADIUS,
  _POWERUP_NUM,
} PowerUpType;

typedef struct {
  Animation *animation;
} PowerUp;

typedef struct {
  int x;
  int y;
} Position;

typedef enum {
  NORTH,
  EAST,
  SOUTH,
  WEST,
  _DIRECTION_NUM,
} Direction;

typedef struct {
  int id;
  Position position;
  Position targetPosition;
  float progress;
  Direction facing;
} Entity;

typedef struct {
  Entity entity;
  double startTime;
  float speed;
  Animation *animation;
} Explosion;

typedef struct {
  Entity entity;
  double endTime;
  double startTime;
  _Bool isExploded;
  Explosion *explosion[_DIRECTION_NUM];
  Animation *animation;
} Bomb;

#define MAX_PLAYERS 4

typedef enum {
  IDLE,
  WALKING,
  _PLAYER_STATE_NUM,
} PlayerState;

typedef struct {
  Entity entity;
  float speed;
  _Bool isAlive;
  PlayerState state;
  Bomb *bombList[20];
  int bombs;
  int blastRadius;
  Animation *animation[_PLAYER_STATE_NUM];
} Player;

struct Game {
  const char *title;
  GameStateType state;
  void (*stateFunction)(Game *);
  MainMenu *mainMenu;
  PauseMenu *pauseMenu;
  float deltaTime;
  Cell grid[GRID_WIDTH][GRID_HEIGHT];
  Player *player[MAX_PLAYERS];
};

Game *InitGame();

void GameLoop();

void UpdateGameState(Game *game, GameStateType stateType);

// MainMenu
void MenuMoveUp(Game *game);
void MenuMoveDown(Game *game);
void MenuSelectOption(Game *game);

// PauseMenu
void PauseSwitchState(Game *game);

// Player
void MovePlayer(Player *player, Direction direction);
void UpdatePlayerPositionProgress(Player *player);
void UpdatePlayerState(Player *player, PlayerState state);

// Bomb
void PlantBomb(Player *player);
#endif // STATE_H
