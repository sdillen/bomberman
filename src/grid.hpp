#pragma once
#include <random>

struct Position {
  int x{};
  int y{};
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

constexpr int kGridWidth = 15;
constexpr int kGridHeight = 15;

class Grid {
public:
  void init(std::mt19937 &rng);
  bool isCollision(Position pos) const;
  Cell getCell(Position pos) const;
  void setCell(Position pos, CellType type);
  static Position getSpawn(int id);

private:
  Cell grid_[kGridWidth][kGridHeight]{};
  bool isSolidWall(Position pos) const;
  bool isSpawnProtected(Position pos) const;
};
