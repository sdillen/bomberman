#include "grid.hpp"

void Grid::init(std::mt19937 &rng) {
  for (int x = 0; x < kGridWidth; x++) {
    for (int y = 0; y < kGridHeight; y++) {
      Position position = {x, y};
      if (isSolidWall(position)) {
        setCell(position, CellType::SolidWall);
      } else if (isSpawnProtected(position)) {
        setCell(position, CellType::Empty);
      } else {
        if (std::uniform_int_distribution(0, 10)(rng) <= 7) {
          setCell(position, CellType::Destructible);
        }
      }
    };
  };
}

bool Grid::isSolidWall(Position position) const {
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

bool Grid::isSpawnProtected(Position position) const {
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

Position Grid::getSpawn(int id) {
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

bool Grid::isCollision(Position pos) const {
  if (grid_[pos.x][pos.y] == CellType::Empty) {
    return false;
  }
  if (grid_[pos.x][pos.y] == CellType::PowerUp) {
    return false;
  }
  return true;
}

CellType Grid::getCell(Position pos) const {
  return grid_[pos.x][pos.y];
}

void Grid::setCell(Position pos, CellType type) {
  grid_[pos.x][pos.y] = type;
}

std::array<std::vector<Position>, kGridDirections> Grid::blastCells(
    Position origin, int radius) const {
  constexpr int dx[kGridDirections] = {0, 1, 0, -1}; // N, E, S, W
  constexpr int dy[kGridDirections] = {-1, 0, 1, 0};
  std::array<std::vector<Position>, kGridDirections> out;
  for (int d = 0; d < kGridDirections; d++) {
    auto &cells = out[d];
    for (int k = 1; k <= radius; k++) {
      Position p = {origin.x + dx[d] * k, origin.y + dy[d] * k};
      if (p.x < 0 || p.x >= kGridWidth || p.y < 0 || p.y >= kGridHeight)
        break;
      cells.push_back(p);
      CellType type = grid_[p.x][p.y];
      if (type == CellType::SolidWall || type == CellType::Destructible) break;
    }
  }
  return out;
}
