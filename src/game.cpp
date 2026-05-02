#include "game.hpp"
#include "input.hpp"
#include "log.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
#include <random>

static std::mt19937 rng(std::random_device{}());
// Game state functions
void mainMenuState(Game *game);
void charSelectMenuState(Game *game);
void runningCountdownState(Game *game);
void runningState(Game *game);
void pauseState(Game *game);
void exitState(Game *game);

// Grid functions
void initGrid(Cell grid[kGridWidth][kGridHeight], Game *game);
bool isSolidWall(Position position);
bool isSpawnProtected(Position position);
Position getSpawn(int id);
void updateCell(Position position, CellType cellType, Game *game);
bool isCollision(Position next, Game *game);

// Position functions
bool isEqPos(Position p1, Position p2);

// Player functions
std::unique_ptr<Player> initPlayer(int id);

// Bomb
void createExplosion(Bomb *bomb, int blastRadius);
Bomb *getBomb(Position pos, Game *game);

// Destructible
void breakDestructibel(Position pos, Game *game);

// Items
void checkPlayerOnPowerUp(Player *player, Game *game);
void collectPowerUp(Position pos, Player *player, Game *game);

Game *InitGame() {
  // Initialize core game
  Game *game = new Game{};
  if (game == nullptr) {
    LOG_ERROR("Allocation of game failed!", nullptr);
  }
  game->title = "Bomberman";
  // Set init game state
  UpdateGameState(game, GameStateType::MainMenu);
  // Initialize main menu
  game->mainMenu = std::make_unique<MainMenu>();
  game->mainMenu->options[0] = "Neues Spiel";
  game->mainMenu->options[1] = "Beenden";
  // Initialize pause menu
  game->pauseMenu = std::make_unique<PauseMenu>();
  game->pauseMenu->title = "Pause";
  // Initialize character select menu
  game->charSelectMenu = std::make_unique<CharSelectMenu>();
  game->charSelectMenu->title = "Wähle deinen Character";
  // Countdown
  game->countdown = 3;
  // Initialize grid
  initGrid(game->grid, game);
  // Initialize players
  for (int i = 0; i < kMaxPlayers; i++) {
    game->player[i] = initPlayer(i);
  }
  return game;
}

void initGrid(Cell grid[kGridWidth][kGridHeight], Game *game) {
  for (int x = 0; x < kGridWidth; x++) {
    for (int y = 0; y < kGridHeight; y++) {
      Position position = {x, y};
      if (isSolidWall(position)) {
        updateCell(position, CellType::SolidWall, game);
      } else if (isSpawnProtected(position)) {
        updateCell(position, CellType::Empty, game);
      } else {
        // 80% Chance to create a destructible
        if (std::uniform_int_distribution(0, 10)(rng) <= 7) {
          updateCell(position, CellType::Destructible, game);
        }
      }
    };
  };
}

bool isSolidWall(Position position) {
  if (position.x == 0) {
    return true;
  } else if (position.x == kGridWidth - 1) {
    return true;
  }
  if (position.y == 0) {
    return true;
  } else if (position.y == kGridHeight - 1) {
    return true;
  }
  if (position.x % 2 == 0 && position.y % 2 == 0) {
    return true;
  }
  return false;
}

bool isSpawnProtected(Position position) {
  if ((position.x == 1 || position.x == kGridWidth - 2) &&
      (position.y <= 3 || position.y >= kGridHeight - 4)) {
    return true;
  }
  if ((position.y == 1 || position.y == kGridHeight - 2) &&
      (position.x <= 3 || position.x >= kGridWidth - 4)) {
    return true;
  }
  return false;
}

Position getSpawn(int id) {
  // 4 possible spawn locations
  // this will support kMaxPlayers = 4
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

void updateCell(Position position, CellType cellType, Game *game) {
  game->grid[position.x][position.y].type = cellType;
}

bool isCollision(Position next, Game *game) {
  if (game->grid[next.x][next.y].type == CellType::Empty) {
    return false;
  }
  if (game->grid[next.x][next.y].type == CellType::PowerUp) {
    return false;
  }
  return true;
}

bool isEqPos(Position p1, Position p2) {
  if (p1.x == p2.x && p1.y == p2.y) {
    return true;
  }
  return false;
}

std::unique_ptr<Player> initPlayer(int id) {
  auto player = std::make_unique<Player>();
  player->entity.id = id;
  player->entity.position = getSpawn(id);
  player->entity.targetPosition = player->entity.position;
  switch (id) {
  case (0):
  case (3):
    player->entity.facing = Direction::East;
    break;
  case (1):
  case (2):
    player->entity.facing = Direction::West;
    break;
  }
  player->isAlive = true;
  player->speed = 5;
  player->blastRadius = 3;
  player->bombs = 1;
  return player;
}

void MovePlayer(Player *player, Direction direction, Game *game) {
  // Keinen Animationsabbruch
  if (player->state == PlayerState::Idle) {
    Position targetPosition = player->entity.targetPosition;
    switch (direction) {
    case Direction::North:
      targetPosition.y -= 1;
      if (!isCollision(targetPosition, game)) {
        player->entity.targetPosition = targetPosition;
      }
      break;
    case Direction::East:
      targetPosition.x += 1;
      if (!isCollision(targetPosition, game)) {
        player->entity.targetPosition = targetPosition;
        player->entity.facing = Direction::East;
      }
      break;
    case Direction::South:
      targetPosition.y += 1;
      if (!isCollision(targetPosition, game)) {
        player->entity.targetPosition = targetPosition;
      }
      break;
    case Direction::West:
      targetPosition.x -= 1;
      if (!isCollision(targetPosition, game)) {
        player->entity.targetPosition = targetPosition;
        player->entity.facing = Direction::West;
      }
      break;
    default:
      break;
    }
    player->state = PlayerState::Walking;
  }
};

void UpdatePlayerPositionProgress(Player *player, Game *game) {
  if (player->state == PlayerState::Walking) {
    player->entity.progress += game->deltaTime * player->speed;
    if (player->entity.progress >= 1.0f) {
      player->entity.progress = 0;
      player->entity.position = player->entity.targetPosition;
      UpdatePlayerState(player, PlayerState::Idle);
    }
  }
}

void UpdatePlayerState(Player *player, PlayerState state) {
  player->state = state;
}

void PlantBomb(Player *player, Game *game) {
  if (static_cast<int>(player->bombList.size()) >= player->bombs) {
    return;
  }
  auto &pos = player->entity.position;
  if (game->grid[pos.x][pos.y].type == CellType::Bomb) {
    return;
  }
  LOG_INFO("Bomb planted", nullptr);
  auto bomb = std::make_unique<Bomb>();
  bomb->entity.position = pos;
  updateCell(pos, CellType::Bomb, game);
  bomb->startTime = GetTime();
  bomb->endTime = bomb->startTime + 3;
  bomb->animation = CreateAnimation(bombSparkFrames, 0.1f);
  player->bombList.push_back(std::move(bomb));
};

void UpdateBombTimer(Player *player, Game *game) {
  for (auto &bomb : player->bombList) {
    if (bomb != nullptr && bomb->endTime <= GetTime() && bomb->endTime != 0) {
      createExplosion(bomb.get(), player->blastRadius);
      bomb->startTime = 0;
      bomb->endTime = 0;
      updateCell(bomb->entity.position, CellType::Empty, game);
    }
  }
}

void RemoveExplodedBombs(Player *player) {
  std::erase_if(player->bombList, [](const auto &bomb) {
    return bomb != nullptr && bomb->isExploded;
  });
}

void createExplosion(Bomb *bomb, int blastRadius) {
  double startTime = GetTime();
  Position pos = bomb->entity.position;
  for (int i = 0; i < kDirectionCount; i++) {
    auto explosion = std::make_unique<Explosion>();
    explosion->speed = 6;
    explosion->entity.position = pos;
    explosion->startTime = startTime;
    explosion->animation =
        CreateAnimation(explosionBlastFrames, 0.1f);
    switch (static_cast<Direction>(i)) {
    case Direction::North:
      explosion->entity.targetPosition = (Position){pos.x, pos.y - blastRadius};
      break;
    case Direction::East:
      explosion->entity.targetPosition = (Position){pos.x + blastRadius, pos.y};
      break;
    case Direction::South:
      explosion->entity.targetPosition = (Position){pos.x, pos.y + blastRadius};
      break;
    case Direction::West:
      explosion->entity.targetPosition = (Position){pos.x - blastRadius, pos.y};
      break;
    }
    bomb->explosion[i] = std::move(explosion);
  }
}

void updateExplosionProgress(Player *player, Game *game) {
  for (auto &bomb : player->bombList) {
    if (bomb != nullptr && bomb->endTime == 0) {
      int nullExplosions = 0;
      for (int j = 0; j < kDirectionCount; j++) {
        if (bomb->explosion[j] != nullptr) {
          Explosion *explosion = bomb->explosion[j].get();
          float progress = game->deltaTime * explosion->speed;
          float relPos =
              explosion->entity.progress * (player->blastRadius + 1);
          for (int k = 0; k <= player->blastRadius; k++) {
            if (relPos >= k) {
              Position cellPos;
              switch (static_cast<Direction>(j)) {
              case Direction::North:
                cellPos = (Position){explosion->entity.position.x,
                                     explosion->entity.position.y - k};
                break;
              case Direction::East:
                cellPos = (Position){explosion->entity.position.x + k,
                                     explosion->entity.position.y};
                break;
              case Direction::South:
                cellPos = (Position){explosion->entity.position.x,
                                     explosion->entity.position.y + k};
                break;
              case Direction::West:
                cellPos = (Position){explosion->entity.position.x - k,
                                     explosion->entity.position.y};
                break;
              }
              Cell cell = game->grid[cellPos.x][cellPos.y];
              if (cell.type == CellType::Destructible) {
                breakDestructibel(cellPos, game);
                bomb->explosion[j] = nullptr;
                break;
              }
              if (cell.type == CellType::SolidWall) {
                bomb->explosion[j] = nullptr;
                break;
              }
              if (cell.type == CellType::Bomb) {
                Bomb *otherBomb = getBomb(cellPos, game);
                if (otherBomb != nullptr) {
                  LOG_INFO("trigger Bomb: %d.%d", cellPos.x, cellPos.y);
                  otherBomb->endTime = GetTime();
                }
              };
              for (int l = 0; l < kMaxPlayers; l++) {
                Player *player = game->player[l].get();
                  if (player->entity.position.x == cellPos.x &&
                      player->entity.position.y == cellPos.y) {
                    bomb->explosion[j] = nullptr;
                    player->isAlive = false;
                    break;
                  }
                }
              }
            }
            explosion->entity.progress += progress;
            if (explosion->entity.progress >= 1) {
              bomb->explosion[j] = nullptr;
            }
          } else {
            nullExplosions++;
            if (nullExplosions == kDirectionCount) {
              bomb->isExploded = 1;
            }
          }
        }
      }
    }
  }

void checkPlayerAlive(Player *player, Game *game) {
  if (player->isAlive == false) {
    player->state = PlayerState::Death;
  }
}

void breakDestructibel(Position pos, Game *game) {
  if (std::uniform_int_distribution(0, 10)(rng) < 2) {
    updateCell(pos, CellType::PowerUp, game);
  } else {
    updateCell(pos, CellType::Empty, game);
  }
}

Bomb *getBomb(Position pos, Game *game) {
  for (int i = 0; i < kMaxPlayers; i++) {
    for (auto &bomb : game->player[i]->bombList) {
      if (bomb != nullptr && isEqPos(pos, bomb->entity.position)) {
        return bomb.get();
      }
    }
  }
  return nullptr;
}

void checkPlayerOnPowerUp(Player *player, Game *game) {
  Position pos = player->entity.position;
  Cell cell = game->grid[pos.x][pos.y];
  if (cell.type == CellType::PowerUp) {
    collectPowerUp(pos, player, game);
  }
}

void collectPowerUp(Position pos, Player *player, Game *game) {
  int r = std::uniform_int_distribution(0, kPowerUpCount - 1)(rng);
  switch (static_cast<PowerUpType>(r)) {
  case PowerUpType::Speed:
    printf("speed collected\n");
    player->speed++;
    break;
  case PowerUpType::Bomb:
    printf("bomb collected\n");
    player->bombs++;
    break;
  case PowerUpType::BlastRadius:
    printf("blast radius collected\n");
    player->blastRadius++;
    break;
  }
  updateCell(pos, CellType::Empty, game);
}

void GameLoop(Game *game) {
  while (game->state != GameStateType::Exit) {
    HandleInput(game);
    game->stateFunction(game);
    Render(game);
  }
};

void UpdateGameState(Game *game, GameStateType stateType) {
  // Setze den Game Zustand und setze die zugehörige Funktion
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
    for (int i = 0; i < kMaxPlayers; i++) {
      if (i == 0) {
        SetCharAnimationSet(game->charSelectMenu->selectedOption,
                            game->player[i].get());
      } else {
        int char_id = std::uniform_int_distribution(0, kCharacters - 1)(rng);
        SetCharAnimationSet(char_id, game->player[i].get());
      }
    }
    UpdateGameState(game, GameStateType::RunningCountdown);
  }
}

void runningCountdownState(Game *game) {
  LOG_DEBUG("runningCountdownState", nullptr);
  game->deltaTime = GetFrameTime();
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->player[i].get();
    auto *anim = player->animation[static_cast<int>(PlayerState::Spawn)].get();
    if (anim && anim->currentFrame == static_cast<int>(anim->frames.size()) - 1) {
      UpdatePlayerState(player, PlayerState::Idle);
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
  for (int i = 0; i < kMaxPlayers; i++) {
    Player *player = game->player[i].get();
    LOG_DEBUG("%i runningState: UpdatePlayerPositionProgress", i);
    UpdatePlayerPositionProgress(player, game);
    LOG_DEBUG("%i runningState: UpdateBombTimer", i);
    UpdateBombTimer(player, game);
    LOG_DEBUG("%i runningState: updateExplosionProgress", i);
    updateExplosionProgress(player, game);
    LOG_DEBUG("%i runningState: RemoveExplodedBombs", i);
    RemoveExplodedBombs(player);
    LOG_DEBUG("%i runningState: checkPlayerOnPowerUp", i);
    checkPlayerOnPowerUp(player, game);
    LOG_DEBUG("%i runningState: checkPlayerAlive", i);
    checkPlayerAlive(player, game);
  }
}

void pauseState(Game *game) {
  if (!game->pauseMenu->isActive) {
    UpdateGameState(game, GameStateType::Running);
  }
}

void exitState(Game *game) {}

void DestroyGame(Game *game) {
  delete game;
}

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
