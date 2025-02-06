#include "game.h"
#include "grid.h"
#include <raylib.h>

static Image image;
static Texture2D character;

void initRenderer() {
  image = LoadImage("./assets/char/idle000.png");
  character = LoadTextureFromImage(image);
  UnloadImage(image);
}

void renderMainMenu(Game *game);
void renderRunning(Game *game);
void renderPauseMenu(Game *game);

void Render(Game *game) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  switch (game->state) {
  case MAIN_MENU:
    renderMainMenu(game);
    break;
  case RUNNING:
    renderRunning(game);
    break;
  case PAUSE_MENU:
    renderRunning(game);
    renderPauseMenu(game);
    break;
  case EXIT:
    break;
  }
  EndDrawing();
}

void renderMainMenu(Game *game) {
  int fontSize = 20;
  int screenWidth = GetScreenWidth();

  DrawText(game->title,
           screenWidth / 2 - MeasureText(game->title, fontSize * 2) / 2, 50,
           fontSize * 2, BLACK);

  for (int i = 0; i < MENU_OPTIONS; i++) {
    int textWidth = MeasureText(game->mainMenu->options[i], fontSize);
    int x = screenWidth / 2 - textWidth / 2;
    int y = 150 + i * (fontSize + 15);

    if (i == game->mainMenu->selectedOption) {
      DrawText(game->mainMenu->options[i], x, y, fontSize, RED);
    } else {
      DrawText(game->mainMenu->options[i], x, y, fontSize, DARKGRAY);
    }
  }
}
void renderRunning(Game *game) {
  Vector2 offset = {GetScreenWidth() % (TILE_SIZE * GRID_WIDTH) / 2,
                    GetScreenHeight() % (TILE_SIZE * GRID_HEIGHT) / 2};
  Vector2 size = {TILE_SIZE, TILE_SIZE};

  DrawFPS(10, 10);

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      Cell *cell = getCell((Coordinates){x, y});
      Vector2 position = {TILE_SIZE * x + offset.x, TILE_SIZE * y + offset.y};
      switch (cell->type) {
      case CELL_EMTPY:
        DrawRectangleV(position, size, WHITE);
        break;
      case CELL_SOLID_WALL:
        DrawRectangleV(position, size, BLACK);
        break;
      case CELL_BOMB:
        DrawRectangleV(position, size, RED);
        break;
      case CELL_PLAYER:
        DrawTextureEx(character, position, 0, 0.5f, WHITE);
        break;
      default:
        break;
      }
    }
  }
}

void renderPauseMenu(Game *game) {
  int fontSize = 20;
  int screenWidth = GetScreenWidth();
  Color overlayColor = Fade(BLACK, 0.5f);
  DrawRectangle(0, 0, screenWidth, GetScreenHeight(), overlayColor);
  DrawText(game->pauseMenu->title,
           screenWidth / 2 - MeasureText(game->pauseMenu->title, fontSize) / 2,
           50, fontSize, WHITE);
}
