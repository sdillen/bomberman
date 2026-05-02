#include "bomb_manager.hpp"
#include "explosion.hpp"
#include "logger.hpp"
#include "player_manager.hpp"
#include "renderer.hpp"
#include <raylib.h>
#include <algorithm>

bool BombManager::plantBomb(Player &owner, Grid &grid,
                            double currentTime) {
  if (getBombCountForPlayer(owner.getId()) >= owner.getBombs()) {
    return false;
  }
  Position pos = owner.getPosition();
  if (grid.getCell(pos).type == CellType::Bomb) {
    return false;
  }
  LOG_INFO("Bomb planted", nullptr);
  grid.setCell(pos, CellType::Bomb);
  auto bomb = std::make_unique<Bomb>(pos, owner.getBlastRadius(), currentTime);
  bomb->setAnimation(
      std::make_unique<Animation>(SharedFramesTag{}, Renderer::getBombSparkFrames(), 0.1f));
  bombs_.push_back(std::move(bomb));
  return true;
}

void BombManager::updateAll(PlayerManager &playerManager, float deltaTime,
                            Grid &grid, std::mt19937 &rng) {
  for (auto &bomb : bombs_) {
    if (bomb == nullptr) continue;

    if (bomb->isTimerExpired(GetTime())) {
      grid.setCell(bomb->getPosition(), CellType::Empty);
      createExplosion(bomb.get());
      bomb->setEndTime(0);
    }

    if (bomb->getEndTime() == 0) {
      updateBombExplosion(bomb.get(), playerManager, deltaTime, grid, rng);
    }
  }

  std::erase_if(bombs_, [](const auto &bomb) {
    return bomb != nullptr && bomb->isExploded();
  });
}

void BombManager::createExplosion(Bomb *bomb) {
  double startTime = GetTime();
  Position pos = bomb->getPosition();
  int blastRadius = bomb->getBlastRadius();
  for (int i = 0; i < kDirectionCount; i++) {
    auto explosion = std::make_unique<Explosion>(
        pos, static_cast<Direction>(i), blastRadius, startTime, 6.0f);
    explosion->setAnimation(
        std::make_unique<Animation>(SharedFramesTag{}, Renderer::getExplosionBlastFrames(), 0.1f));
    bomb->getExplosions()[i] = std::move(explosion);
  }
}

bool isEqPos(Position p1, Position p2) {
  return p1.x == p2.x && p1.y == p2.y;
}

void BombManager::updateBombExplosion(Bomb *bomb,
                                      PlayerManager &playerManager,
                                      float deltaTime, Grid &grid,
                                      std::mt19937 &rng) {
  int nullExplosions = 0;
  int blastRadius = bomb->getBlastRadius();

  for (int j = 0; j < kDirectionCount; j++) {
    auto &explosion = bomb->getExplosions()[j];
    if (explosion == nullptr) {
      nullExplosions++;
      continue;
    }

    float dt = deltaTime;
    float relPos = explosion->getProgress() * (blastRadius + 1);
    bool cleared = false;

    for (int k = 0; k <= blastRadius && !cleared; k++) {
      if (relPos >= k) {
        Position origin = explosion->getOrigin();
        Position cellPos;
        switch (explosion->getDirection()) {
        case Direction::North:
          cellPos = {origin.x, origin.y - k};
          break;
        case Direction::East:
          cellPos = {origin.x + k, origin.y};
          break;
        case Direction::South:
          cellPos = {origin.x, origin.y + k};
          break;
        case Direction::West:
          cellPos = {origin.x - k, origin.y};
          break;
        }
        Cell cell = grid.getCell(cellPos);
        if (cell.type == CellType::Destructible) {
          if (std::uniform_int_distribution(0, 10)(rng) < 2) {
            grid.setCell(cellPos, CellType::PowerUp);
          } else {
            grid.setCell(cellPos, CellType::Empty);
          }
          bomb->clearExplosion(j);
          cleared = true;
          break;
        }
        if (cell.type == CellType::SolidWall) {
          bomb->clearExplosion(j);
          cleared = true;
          break;
        }
        if (cell.type == CellType::Bomb) {
          Bomb *otherBomb = findBomb(cellPos);
          if (otherBomb != nullptr) {
            LOG_INFO("trigger Bomb: %d.%d", cellPos.x, cellPos.y);
            otherBomb->setEndTime(GetTime());
          }
        }
        for (int l = 0; l < kMaxPlayers && !cleared; l++) {
          Player *player = playerManager.getPlayer(l);
          if (isEqPos(player->getPosition(), cellPos)) {
            bomb->clearExplosion(j);
            player->kill();
            cleared = true;
            break;
          }
        }
      }
    }
    if (!cleared) {
      explosion->updateProgress(dt);
      if (explosion->isFinished()) {
        bomb->clearExplosion(j);
      }
    }
  }

  if (nullExplosions == kDirectionCount) {
    bomb->markExploded();
  }
}

Bomb *BombManager::findBomb(Position pos) const {
  for (auto &bomb : bombs_) {
    if (bomb != nullptr && isEqPos(pos, bomb->getPosition())) {
      return bomb.get();
    }
  }
  return nullptr;
}

int BombManager::getBombCountForPlayer(int playerId) const {
  int count = 0;
  for (auto &bomb : bombs_) {
    if (bomb != nullptr) count++;
  }
  return count;
}
