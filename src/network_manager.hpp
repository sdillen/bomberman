#pragma once
#include "packet.hpp"
#include <enet/enet.h>
#include <string>
#include <functional>

constexpr int kServerPort = 8888;
constexpr int kMaxClients = 3;

class NetworkManager {
public:
  static bool init();
  static void deinit();

  static bool startServer();
  static bool connectToServer(const std::string &address);
  static void disconnect();

  static void pollEvents();

  static void sendPlayerInput(int8_t direction, uint8_t plantBomb);
  static void sendCharSelect(uint8_t playerId, uint8_t charIndex,
                              int peerIndex = -1);

  static void broadcastGameState(const class Game *game);
  static void sendAssignment(uint8_t clientPlayerId,
                             uint8_t numHumanPlayers,
                             const uint8_t charIds[4],
                             int peerIndex = -1);

  static bool receivePlayerInput(int &outPlayerId, int &outDirection,
                                  uint8_t &outPlantBomb);
  static bool receiveCharSelect(uint8_t &outPlayerId, uint8_t &outCharIndex);
  static bool receiveAssignment(uint8_t &outClientPlayerId,
                                uint8_t &outNumHumanPlayers,
                                uint8_t outCharIds[4]);
  static bool receiveGameState(class Game *game);

  static bool isServer() { return server_ != nullptr; }
  static bool isClient() { return client_ != nullptr && peer_ != nullptr; }
  static bool isConnected();

  static int getNumConnectedPeers();

private:
  static ENetHost *server_;
  static ENetHost *client_;
  static ENetPeer *peer_;
  static bool initialized_;

  static bool receivePacketFromHost(ENetHost *host, uint8_t *&outData,
                                     size_t &outLength, ENetPeer *&outSender);
  static void sendPacket(void *data, size_t size, ENetPeer *target,
                         uint8_t channel, bool reliable);
};
