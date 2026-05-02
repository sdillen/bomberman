#pragma once
#include "player.hpp"
#include <memory>
#include <array>

constexpr int kMaxPlayers = 4;

class PlayerManager {
public:
  void initAll();
  void updateMovement(float deltaTime);
  void checkAllAlive();

  Player *getPlayer(int index);
  const Player *getPlayer(int index) const;
  std::array<std::unique_ptr<Player>, kMaxPlayers> &getPlayers();
  const std::array<std::unique_ptr<Player>, kMaxPlayers> &getPlayers() const;

private:
  std::array<std::unique_ptr<Player>, kMaxPlayers> players_;
};
