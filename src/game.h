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

struct Game {
  const char *title;
  GameStateType state;
  void (*stateFunction)(Game *);
  MainMenu *mainMenu;
  PauseMenu *pauseMenu;
};

Game *InitGame();

void GameLoop(Game *game);

void UpdateGameState(Game *game, GameStateType stateType);

void MenuMoveUp(Game *game);
void MenuMoveDown(Game *game);
void MenuSelectOption(Game *game);

void PauseSwitchState(Game *game);

typedef struct {
  int id;
  Vector2 position;
  float speed;
} Entity;

typedef struct {
  Entity entity;
  _Bool isAlive;
} Player;

typedef struct {
  Entity entity;
} Bomb;

typedef struct {
  Entity entity;
} Destructible;

GameStateType updateMenu();

#endif // STATE_H
