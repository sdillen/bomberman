#include "bot_controller.hpp"
#include "player_manager.hpp"
#include "bomb_manager.hpp"
#include "grid.hpp"
#include "player.hpp"
#include <queue>
#include <algorithm>
#include <climits>

BotController::BotState BotController::botStates_[3];
bool BotController::initialized_ = false;

namespace {
struct Reach {
  bool visited[kGridWidth][kGridHeight] = {};
  int dist[kGridWidth][kGridHeight] = {};
  Position parent[kGridWidth][kGridHeight] = {};
};

// BFS von start aus. wallAware=false ignoriert Kollisionen (reine Geometrie).
Reach flood(const Grid &grid, Position start, int maxDepth, bool wallAware) {
  Reach r;
  std::queue<Position> q;
  q.push(start);
  r.visited[start.x][start.y] = true;
  r.dist[start.x][start.y] = 0;
  constexpr int dx[] = {0, 1, 0, -1};
  constexpr int dy[] = {-1, 0, 1, 0};
  while (!q.empty()) {
    Position cur = q.front();
    q.pop();
    if (maxDepth >= 0 && r.dist[cur.x][cur.y] >= maxDepth) continue;
    for (int i = 0; i < 4; i++) {
      Position next = {cur.x + dx[i], cur.y + dy[i]};
      if (next.x < 0 || next.x >= kGridWidth || next.y < 0 ||
          next.y >= kGridHeight)
        continue;
      if (r.visited[next.x][next.y]) continue;
      if (wallAware && grid.isCollision(next)) continue;
      r.visited[next.x][next.y] = true;
      r.dist[next.x][next.y] = r.dist[cur.x][cur.y] + 1;
      r.parent[next.x][next.y] = cur;
      q.push(next);
    }
  }
  return r;
}

std::vector<Position> pathTo(const Reach &r, Position start, Position goal) {
  std::vector<Position> path;
  Position p = goal;
  while (!(p.x == start.x && p.y == start.y)) {
    path.push_back(p);
    p = r.parent[p.x][p.y];
  }
  std::reverse(path.begin(), path.end());
  return path;
}
} // namespace

static Direction directionFromTo(Position from, Position to) {
    if (to.x == from.x && to.y < from.y) return Direction::North;
    if (to.x > from.x && to.y == from.y) return Direction::East;
    if (to.x == from.x && to.y > from.y) return Direction::South;
    if (to.x < from.x && to.y == from.y) return Direction::West;
    return Direction::North;
}

void BotController::updateAll(PlayerManager& playerManager, BombManager& bombManager,
                               Grid& grid, float deltaTime, double currentTime,
                               int numHumanPlayers, std::mt19937& rng) {
    if (!initialized_) {
        auto dist = std::uniform_int_distribution(0, 9);
        for (int i = 0; i < 3; i++) {
            botStates_[i].decisionTimer = static_cast<float>(dist(rng)) / 100.0f;
        }
        initialized_ = true;
    }

    for (int i = 0; i < 3; i++) {
        int playerId = i + 1;
        if (playerId < numHumanPlayers) continue;
        Player* player = playerManager.getPlayer(playerId);
        if (player != nullptr) {
            updateBot(i, *player, playerManager, bombManager, grid, deltaTime, currentTime, rng);
        }
    }
}

void BotController::updateBot(int botIndex, Player& player, PlayerManager& playerManager,
                               BombManager& bombManager, Grid& grid, float deltaTime,
                               double currentTime, std::mt19937& rng) {
    if (!player.isAlive()) return;
    if (player.getState() != PlayerState::Idle) return;

    BotState& state = botStates_[botIndex];
    Position playerPos = player.getPosition();
    auto blastTiles = getBlastTiles(bombManager, grid);
    bool danger = isInDanger(player, blastTiles);

    if (danger) {
        auto fleeStep = findSafeFleeStep(player, grid, blastTiles);
        if (fleeStep.size() >= 1) {
            Direction dir = directionFromTo(playerPos, fleeStep[0]);
            player.move(dir, grid);
            state.holdTimer = 1.0f;
            state.currentGoal = Goal{};
            state.path.clear();
            state.pathIndex = 0;
            return;
        }
        return;
    }

    if (state.holdTimer > 0.0f) {
        state.holdTimer -= deltaTime;
        return;
    }

    state.decisionTimer -= deltaTime;
    if (state.decisionTimer > 0) {
        if (state.pathIndex < state.path.size()) {
            Position next = state.path[state.pathIndex];
            Direction dir = directionFromTo(playerPos, next);
            player.move(dir, grid);
            if (player.getState() == PlayerState::Walking) {
                state.pathIndex++;
            }
        }
        return;
    }

    state.decisionTimer = 0.15f + static_cast<float>(std::uniform_int_distribution(0, 4)(rng)) * 0.01f;

    if (isBombNearby(player, bombManager, grid)) {
        return;
    }

    Goal goal = findBombGoal(player, playerManager, bombManager, grid, currentTime);

    if (goal.type == GoalType::None) {
        goal = findChaseGoal(player, playerManager);
    }

    if (goal.type == GoalType::None) {
        goal = findPowerUpGoal(player, grid);
    }

    if (goal.type == GoalType::None) {
        goal = findExploreGoal(player, grid);
    }

    if (goal.type == GoalType::PlantBomb) {
        bombManager.plantBomb(player, grid, currentTime);
        for (int i = 0; i < 4; i++) {
            Direction d = static_cast<Direction>(i);
            player.move(d, grid);
            if (player.getState() == PlayerState::Walking) break;
        }
        state.currentGoal = Goal{};
        state.path.clear();
        state.pathIndex = 0;
        return;
    }

    state.currentGoal = goal;
    state.path.clear();
    state.pathIndex = 0;

    if (goal.type != GoalType::None) {
        state.path = findPath(grid, playerPos, goal.position);
    }

    if (state.path.empty()) {
        randomMove(player, grid, rng);
        return;
    }

    Position next = state.path[0];
    Direction dir = directionFromTo(playerPos, next);
    player.move(dir, grid);
    if (player.getState() == PlayerState::Walking) {
        state.pathIndex = 1;
    }
}

bool BotController::isInDanger(const Player& player, const std::vector<Position>& blastTiles) {
    Position pos = player.getPosition();
    for (auto& tile : blastTiles) {
        if (tile.x == pos.x && tile.y == pos.y) return true;
    }
    return false;
}

void BotController::randomMove(Player& player, const Grid& grid, std::mt19937& rng) {
    int offset = std::uniform_int_distribution(0, 3)(rng);
    Direction dirs[] = {Direction::North, Direction::East, Direction::South, Direction::West};

    for (int i = 0; i < 4; i++) {
        Direction d = dirs[(offset + i) % 4];
        player.move(d, grid);
        if (player.getState() == PlayerState::Walking) return;
    }
}

std::vector<Position> BotController::findSafeFleeStep(const Player& player, const Grid& grid,
                                                       const std::vector<Position>& blastTiles) {
    Position start = player.getPosition();
    Reach r = flood(grid, start, -1, true);

    Position best{};
    int bestDist = INT_MAX;
    for (int x = 0; x < kGridWidth; x++) {
        for (int y = 0; y < kGridHeight; y++) {
            if (!r.visited[x][y]) continue;
            if (x == start.x && y == start.y) continue;
            bool safe = true;
            for (auto &tile : blastTiles) {
                if (tile.x == x && tile.y == y) { safe = false; break; }
            }
            if (safe && r.dist[x][y] < bestDist) {
                bestDist = r.dist[x][y];
                best = {x, y};
            }
        }
    }
    if (bestDist == INT_MAX) return {};
    return pathTo(r, start, best);
}

BotController::Goal BotController::findBombGoal(const Player& player,
                                  const PlayerManager& playerManager,
                                  BombManager& bombManager,
                                  Grid& grid, double currentTime) {
    (void)currentTime;
    if (bombManager.getBombCountForPlayer(player.getId()) >= player.getBombs()) {
        return {};
    }
    if (!hasEscapeRoute(player.getPosition(), player.getBlastRadius(), grid)) {
        return {};
    }
    if (isAdjacentToEnemyOrWall(player, playerManager, grid)) {
        return {GoalType::PlantBomb, player.getPosition()};
    }
    return {};
}

BotController::Goal BotController::findChaseGoal(const Player& player,
                                   const PlayerManager& playerManager) {
    Position myPos = player.getPosition();
    int bestDist = 999;
    Position bestPos{};

    for (int i = 0; i < kMaxPlayers; i++) {
        const Player* other = playerManager.getPlayer(i);
        if (other == nullptr || !other->isAlive()) continue;
        if (other->getId() == player.getId()) continue;

        Position otherPos = other->getPosition();
        int dist = std::abs(myPos.x - otherPos.x) + std::abs(myPos.y - otherPos.y);
        if (dist < bestDist) {
            bestDist = dist;
            bestPos = otherPos;
        }
    }

    if (bestDist < 999) {
        return {GoalType::ChaseEnemy, bestPos};
    }
    return {};
}

BotController::Goal BotController::findPowerUpGoal(const Player& player,
                                     const Grid& grid) {
    Position myPos = player.getPosition();
    int bestDist = 999;
    Position bestPos{};

    for (int x = 0; x < kGridWidth; x++) {
        for (int y = 0; y < kGridHeight; y++) {
            CellType type = grid.getCell({x, y});
            if (type == CellType::PowerUp) {
                int dist = std::abs(myPos.x - x) + std::abs(myPos.y - y);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestPos = {x, y};
                }
            }
        }
    }

    if (bestDist < 999) {
        return {GoalType::CollectPowerUp, bestPos};
    }
    return {};
}

BotController::Goal BotController::findExploreGoal(const Player& player,
                                     const Grid& grid) {
    Position myPos = player.getPosition();
    int bestDist = 999;
    Position bestPos{};

    for (int x = 0; x < kGridWidth; x++) {
        for (int y = 0; y < kGridHeight; y++) {
            CellType type = grid.getCell({x, y});
            if (type != CellType::Destructible) continue;

            constexpr int dx[] = {0, 1, 0, -1};
            constexpr int dy[] = {-1, 0, 1, 0};

            for (int i = 0; i < 4; i++) {
                Position adj = {x + dx[i], y + dy[i]};
                if (adj.x < 0 || adj.x >= kGridWidth || adj.y < 0 || adj.y >= kGridHeight) continue;
                if (grid.isCollision(adj)) continue;

                int dist = std::abs(myPos.x - adj.x) + std::abs(myPos.y - adj.y);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestPos = adj;
                }
            }
        }
    }

    if (bestDist < 999) {
        return {GoalType::Explore, bestPos};
    }
    return {};
}

bool BotController::isBombNearby(const Player& player,
                                  const BombManager& bombManager,
                                  const Grid& grid) {
    Reach r = flood(grid, player.getPosition(), 6, false);
    for (int x = 0; x < kGridWidth; x++) {
        for (int y = 0; y < kGridHeight; y++) {
            if (!r.visited[x][y]) continue;
            for (auto &bomb : bombManager.getBombs()) {
                if (bomb != nullptr && !bomb->isExploded() &&
                    bomb->getPosition().x == x && bomb->getPosition().y == y) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<Position> BotController::findPath(const Grid& grid, Position start, Position goal) {
    if (start.x == goal.x && start.y == goal.y) return {};
    if (goal.x < 0 || goal.x >= kGridWidth || goal.y < 0 || goal.y >= kGridHeight) return {};

    Reach r = flood(grid, start, -1, true);
    if (!r.visited[goal.x][goal.y]) return {};
    return pathTo(r, start, goal);
}

std::vector<Position> BotController::getBlastTiles(const BombManager& bombManager,
                                                    const Grid& grid) {
    std::vector<Position> tiles;

    for (auto& bomb : bombManager.getBombs()) {
        if (bomb == nullptr || bomb->isExploded()) continue;
        auto fronts = grid.blastCells(bomb->getPosition(), bomb->getBlastRadius());
        tiles.push_back(bomb->getPosition());
        for (auto &front : fronts) {
            tiles.insert(tiles.end(), front.begin(), front.end());
        }
    }

    return tiles;
}

bool BotController::isAdjacentToEnemyOrWall(const Player& player,
                                             const PlayerManager& playerManager,
                                             const Grid& grid) {
    Position pos = player.getPosition();

    constexpr int dx[] = {0, 1, 0, -1};
    constexpr int dy[] = {-1, 0, 1, 0};

    for (int i = 0; i < 4; i++) {
        Position adj = {pos.x + dx[i], pos.y + dy[i]};
        if (adj.x < 0 || adj.x >= kGridWidth || adj.y < 0 || adj.y >= kGridHeight) continue;

        CellType type = grid.getCell(adj);
        if (type == CellType::Destructible) return true;

        for (int p = 0; p < kMaxPlayers; p++) {
            const Player* other = playerManager.getPlayer(p);
            if (other == nullptr || !other->isAlive()) continue;
            if (other->getId() == player.getId()) continue;
            Position otherPos = other->getPosition();
            if (otherPos.x == adj.x && otherPos.y == adj.y) {
                return true;
            }
        }
    }

    return false;
}

bool BotController::hasEscapeRoute(Position pos, int blastRadius,
                                    const Grid& grid) {
    Reach r = flood(grid, pos, -1, true);
    for (int x = 0; x < kGridWidth; x++) {
        for (int y = 0; y < kGridHeight; y++) {
            if (r.visited[x][y] && r.dist[x][y] >= blastRadius + 1) return true;
        }
    }
    return false;
}
