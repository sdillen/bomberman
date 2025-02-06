#include "grid.h"
#include <raylib.h>

static Cell grid[GRID_WIDTH][GRID_HEIGHT];
static Coordinates player;

_Bool isSolidWall(Coordinates coord);
_Bool isDestructable(Coordinates coord);
void updateCell(Coordinates coord, CellType newType);
void spawnPlayer(Coordinates coord);
void movePlayer(Coordinates coord, Direction dir);
void initBomb(Coordinates coord);
void updateBombList();

void initGrid() {
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      if (isSolidWall((Coordinates){x, y})) {
        updateCell((Coordinates){x, y}, CELL_SOLID_WALL);
      } else {
        updateCell((Coordinates){x, y}, CELL_EMTPY);
      }
    };
  };
  updateCell((Coordinates){1, 1}, CELL_PLAYER);
}

void updateGrid() {
  if (IsKeyPressed(KEY_W)) {
    movePlayer(player, NORTH);
  }
  if (IsKeyPressed(KEY_D)) {
    movePlayer(player, EAST);
  }
  if (IsKeyPressed(KEY_S)) {
    movePlayer(player, SOUTH);
  }
  if (IsKeyPressed(KEY_A)) {
    movePlayer(player, WEST);
  }
  if (IsKeyPressed(KEY_SPACE)) {
    updateCell(player, CELL_BOMB);
  }
}

_Bool isSolidWall(Coordinates coord) {
  if (coord.x == 0) {
    return 1;
  } else if (coord.x == GRID_WIDTH - 1) {
    return 1;
  }
  if (coord.y == 0) {
    return 1;
  } else if (coord.y == GRID_HEIGHT - 1) {
    return 1;
  }
  if (coord.x % 2 == 0 && coord.y % 2 == 0) {
    return 1;
  }
  return 0;
}

_Bool isDestructable(Coordinates coord) { return 0; }

Cell *getCell(Coordinates coord) { return &grid[coord.x][coord.y]; }

void updateCell(Coordinates coord, CellType newType) {
  Cell *cell = &grid[coord.x][coord.y];
  cell->type = newType;
  if (newType == CELL_PLAYER) {
    player = coord;
  }
}

void movePlayer(Coordinates coord, Direction dir) {
  Coordinates next = coord;
  switch (dir) {
  case NORTH:
    next.y -= 1;
    break;
  case EAST:
    next.x += 1;
    break;
  case SOUTH:
    next.y += 1;
    break;
  case WEST:
    next.x -= 1;
    break;
  }
  if (getCell(next)->type == CELL_EMTPY) {
    if (getCell(coord)->type == CELL_BOMB) {
      updateCell(coord, CELL_BOMB);
    } else {
      updateCell(coord, CELL_EMTPY);
    }
    updateCell(next, CELL_PLAYER);
  }
}
