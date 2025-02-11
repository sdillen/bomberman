#ifndef STATE_H
#define STATE_H
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
} CellType;

typedef struct {
  CellType type;
} Cell;

typedef struct {
  int x;
  int y;
} Position;

typedef enum {
  NORTH,
  EAST,
  SOUTH,
  WEST,
} Direction;

#define DIRECTIONS 4

typedef struct {
  int id;
  Position position;
  Position targetPosition;
  float progress;
  Direction facing;
} Entity;

typedef struct {
  Entity entity;
  int blastRadius;
  double startTime;
  float speed;
} Explosion;

typedef struct {
  Entity entity;
  double endTime;
  double startTime;
  _Bool isExploded;
  Explosion *explosion[DIRECTIONS];
} Bomb;

#define MAX_PLAYERS 2
#define MAX_BOMBS 1

typedef enum {
  IDLE,
  WALKING,
} PlayerState;

typedef struct {
  Entity entity;
  float speed;
  _Bool isAlive;
  PlayerState state;
  Bomb *bombs[MAX_BOMBS];
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

void InitGame();

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
