#pragma once
#include <array>
#include <random>
#include <vector>

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

constexpr int kGridWidth = 15;
constexpr int kGridHeight = 15;
constexpr int kGridDirections = 4;

class Grid {
public:
  void init(std::mt19937 &rng);
  bool isCollision(Position pos) const;
  CellType getCell(Position pos) const;
  void setCell(Position pos, CellType type);
  // Füllt pro Richtung (Index: North, East, South, West) die Blast-Zellen bis
  // vor die erste Mauer/Box; die blockierende Zelle ist enthalten.
  std::array<std::vector<Position>, kGridDirections> blastCells(
      Position origin, int radius) const;
  static Position getSpawn(int id);

private:
  CellType grid_[kGridWidth][kGridHeight]{};
  bool isSolidWall(Position pos) const;
  bool isSpawnProtected(Position pos) const;
};
