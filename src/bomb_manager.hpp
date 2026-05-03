#pragma once
#include "bomb.hpp"
#include "grid.hpp"
#include "player.hpp"
#include <memory>
#include <vector>
#include <random>

class PlayerManager;

class BombManager {
public:
  bool plantBomb(Player &owner, Grid &grid, double currentTime);
  void updateAll(PlayerManager &playerManager, float deltaTime, Grid &grid,
                 std::mt19937 &rng);
  Bomb *findBomb(Position pos) const;
  int getBombCountForPlayer(int playerId) const;
  void clearAll();
  void restoreBomb(Position pos, int ownerId, int blastRadius, double remainingTime);
  void syncBombs(const Position positions[], int ownerIds[], int blastRadii[],
                 double remainingTimes[], int count);
  void startLocalExplosion(Position pos, int blastRadius, const Grid &grid);
  void updateExplosions(float deltaTime);

  std::vector<std::unique_ptr<Bomb>> &getBombs() { return bombs_; }
  const std::vector<std::unique_ptr<Bomb>> &getBombs() const { return bombs_; }

private:
  std::vector<std::unique_ptr<Bomb>> bombs_{};

  void createExplosion(Bomb *bomb);
  void updateBombExplosion(Bomb *bomb, PlayerManager &playerManager,
                           float deltaTime, Grid &grid, std::mt19937 &rng);
};
