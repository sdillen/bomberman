#pragma once
#include "player_manager.hpp"
#include "bomb_manager.hpp"
#include "grid.hpp"
#include <vector>

class BotController {
public:
    static void updateAll(PlayerManager& playerManager, BombManager& bombManager,
                          Grid& grid, float deltaTime, double currentTime,
                          int numHumanPlayers = 1);

private:
    enum class GoalType {
        None,
        Flee,
        PlantBomb,
        ChaseEnemy,
        CollectPowerUp,
        Explore,
    };

    struct Goal {
        GoalType type = GoalType::None;
        Position position{};
    };

    struct BotState {
        Goal currentGoal;
        std::vector<Position> path;
        size_t pathIndex = 0;
        float decisionTimer = 0.0f;
        float holdTimer = 0.0f;
    };

    static BotState botStates_[3];
    static bool initialized_;

    static void updateBot(int botIndex, Player& player, PlayerManager& playerManager,
                          BombManager& bombManager, Grid& grid, float deltaTime, double currentTime);

    static bool isInDanger(const Player& player, const std::vector<Position>& blastTiles);
    static std::vector<Position> findSafeFleeStep(const Player& player, const Grid& grid,
                                                   const std::vector<Position>& blastTiles);
    static Goal findBombGoal(const Player& player, const PlayerManager& playerManager,
                             BombManager& bombManager, Grid& grid, double currentTime);
    static Goal findChaseGoal(const Player& player, const PlayerManager& playerManager);
    static Goal findPowerUpGoal(const Player& player, const Grid& grid);
    static Goal findExploreGoal(const Player& player, const Grid& grid);
    static bool isBombNearby(const Player& player, const BombManager& bombManager);

    static std::vector<Position> findPath(const Grid& grid, Position start, Position goal);
    static std::vector<Position> getBlastTiles(const BombManager& bombManager, const Grid& grid);
    static bool isAdjacentToEnemyOrWall(const Player& player, const PlayerManager& playerManager, const Grid& grid);
    static bool hasEscapeRoute(Position pos, const Grid& grid);
    static void randomMove(Player& player, const Grid& grid);
};
