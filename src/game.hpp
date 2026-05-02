#pragma once
#include "animation.hpp"
#include "grid.hpp"
#include "player.hpp"
#include "player_manager.hpp"
#include "bomb_manager.hpp"
#include <raylib.h>
#include <memory>
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

enum class PowerUpType {
  Speed,
  Bomb,
  BlastRadius,
};

struct PowerUp {
  std::unique_ptr<Animation> animation;
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
  PlayerManager playerManager;
  BombManager bombManager;
  Grid grid;
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



