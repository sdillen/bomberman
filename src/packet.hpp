#pragma once
#include <cstdint>

#pragma pack(push, 1)

enum class PacketType : uint8_t {
  PlayerInput,
  CharSelect,
  Assignment,
  GameState,
};

struct PacketHeader {
  PacketType type;
};

struct PlayerInputPacket {
  int8_t direction;
  uint8_t plantBomb;
};

struct CharSelectPacket {
  uint8_t playerId;
  uint8_t charIndex;
};

struct AssignmentPacket {
  uint8_t clientPlayerId;
  uint8_t numHumanPlayers;
  uint8_t playerCharIds[4];
};

struct PlayerStateData {
  uint8_t id;
  int16_t posX, posY;
  int16_t targetX, targetY;
  float progress;
  float speed;
  uint8_t facing;
  uint8_t isAlive;
  uint8_t state;
  uint8_t bombs;
  uint8_t blastRadius;
};

struct BombStateData {
  int16_t posX, posY;
  float remainingTime;
  uint8_t blastRadius;
  uint8_t ownerId;
  uint8_t active;
};

constexpr int kMaxBombStates = 32;
constexpr int kGridCells = 225;

struct GameStatePacket {
  uint8_t numPlayers;
  uint8_t numBombs;
  PlayerStateData players[4];
  BombStateData bombs[kMaxBombStates];
  uint8_t grid[kGridCells];
};

#pragma pack(pop)
