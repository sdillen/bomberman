#include "player_manager.hpp"

void PlayerManager::initAll() {
  for (int i = 0; i < kMaxPlayers; i++) {
    players_[i] = std::make_unique<Player>(i);
  }
}

void PlayerManager::updateMovement(float deltaTime) {
  for (auto &p : players_) {
    p->updatePositionProgress(deltaTime);
  }
}

void PlayerManager::checkAllAlive() {
  for (auto &p : players_) {
    p->checkAlive();
  }
}

Player *PlayerManager::getPlayer(int index) {
  if (index < 0 || index >= kMaxPlayers) return nullptr;
  return players_[index].get();
}
const Player *PlayerManager::getPlayer(int index) const {
  if (index < 0 || index >= kMaxPlayers) return nullptr;
  return players_[index].get();
}

std::array<std::unique_ptr<Player>, kMaxPlayers> &PlayerManager::getPlayers() {
  return players_;
}

const std::array<std::unique_ptr<Player>, kMaxPlayers> &
PlayerManager::getPlayers() const {
  return players_;
}
