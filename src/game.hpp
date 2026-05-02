#pragma once
#include "util.hpp"
#include <raylib.h>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <string>

enum class GameStateType {
  MainMenu,
  CharSelectMenu,
  RunningCountdown,
  Running,
  PauseMenu,
  Exit,
};

struct Game;

constexpr int kMenuOptions = 2;
constexpr int kPauseOptions = 1;
constexpr int kCharacters = 18;
constexpr int kGridWidth = 15;
constexpr int kGridHeight = 15;
constexpr int kMaxPlayers = 4;
constexpr int kDirectionCount = 4;
constexpr int kPlayerStateCount = 4;
constexpr int kPowerUpCount = 3;

struct MainMenu {
  std::string options[kMenuOptions];
  int selectedOption{};
  bool next{};
};

struct PauseMenu {
  std::string title;
  bool isActive{};
};

struct CharSelectMenu {
  std::string title;
  int selectedOption{};
  Texture2D charTexture[kCharacters]{};
  bool next{};
};

enum class CellType {
  Empty,
  SolidWall,
  Destructible,
  Bomb,
  PowerUp,
};

struct Cell {
  CellType type{};
};

enum class PowerUpType {
  Speed,
  Bomb,
  BlastRadius,
};

struct PowerUp {
  Animation *animation{};
};

struct Position {
  int x{};
  int y{};
};

enum class Direction {
  North,
  East,
  South,
  West,
};

struct Entity {
  int id{};
  Position position;
  Position targetPosition;
  float progress{};
  Direction facing{};
};

struct Explosion {
  Entity entity;
  double startTime{};
  float speed{};
  Animation *animation{};
};

struct Bomb {
  Entity entity;
  double endTime;
  double startTime;
  bool isExploded;
  std::array<std::unique_ptr<Explosion>, kDirectionCount> explosion;
  Animation *animation;
};



enum class PlayerState {
  Spawn,
  Idle,
  Walking,
  Death,
};

struct Player {
  Entity entity;
  float speed;
  bool isAlive;
  PlayerState state;
  std::vector<std::unique_ptr<Bomb>> bombList;
  int bombs;
  int blastRadius;
  std::array<std::unique_ptr<Animation>, kPlayerStateCount> animation;
};

struct Game {
  std::string title;
  GameStateType state{};
  std::function<void(Game *)> stateFunction;
  std::unique_ptr<MainMenu> mainMenu;
  std::unique_ptr<PauseMenu> pauseMenu;
  std::unique_ptr<CharSelectMenu> charSelectMenu;
  float countdown;
  float deltaTime;
  Cell grid[kGridWidth][kGridHeight];
  std::array<std::unique_ptr<Player>, kMaxPlayers> player;
};

Game *InitGame();

void GameLoop(Game *game);

void UpdateGameState(Game *game, GameStateType stateType);
void DestroyGame(Game *game);

// MainMenu
void MenuMoveUp(Game *game);
void MenuMoveDown(Game *game);
void MenuSelectOption(Game *game);

// CharSelectMenu
void NextChar(Game *game);
void PrevChar(Game *game);
void SelectChar(Game *game);

// PauseMenu
void PauseSwitchState(Game *game);

// Player
void MovePlayer(Player *player, Direction direction, Game *game);
void UpdatePlayerPositionProgress(Player *player);
void UpdatePlayerState(Player *player, PlayerState state);

// Bomb
void PlantBomb(Player *player, Game *game);

