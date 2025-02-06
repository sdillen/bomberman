#ifndef GRID_H
#define GRID_H

#include <raylib.h>
typedef enum {
  CELL_EMTPY,
  CELL_SOLID_WALL,
  CELL_DESTRUCTIBLE,
  CELL_PLAYER,
  CELL_BOMB,
} CellType;

typedef struct {
  CellType type;
} Cell;

typedef struct {
  int x;
  int y;
} Coordinates;

typedef enum {
  NORTH,
  EAST,
  SOUTH,
  WEST,
} Direction;

#define GRID_WIDTH 15
#define GRID_HEIGHT 15

#define TILE_SIZE 32

void initGrid();
void renderGrid();
void updateGrid();
Cell *getCell(Coordinates coord);

#endif // GRID_H
