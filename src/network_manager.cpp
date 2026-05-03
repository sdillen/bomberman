#include "network_manager.hpp"
#include "packet.hpp"
#include "logger.hpp"
#include "game.hpp"
#include "player_manager.hpp"
#include "bomb_manager.hpp"
#include "grid.hpp"
#include <cstring>
#include <vector>

ENetHost *NetworkManager::server_ = nullptr;
ENetHost *NetworkManager::client_ = nullptr;
ENetPeer *NetworkManager::peer_ = nullptr;
bool NetworkManager::initialized_ = false;
Position NetworkManager::prevBombPositions_[32] = {};
int NetworkManager::prevBlastRadii_[32] = {};
int NetworkManager::prevBombCount_ = 0;

static std::vector<ENetPeer *> connectedPeers;

bool NetworkManager::init() {
  if (initialized_) return true;
  if (enet_initialize() != 0) {
    LOG_ERROR("ENet initialization failed");
    return false;
  }
  initialized_ = true;
  LOG_INFO("ENet initialized");
  return true;
}

void NetworkManager::deinit() {
  disconnect();
  if (server_) {
    enet_host_destroy(server_);
    server_ = nullptr;
  }
  if (client_) {
    enet_host_destroy(client_);
    client_ = nullptr;
  }
  connectedPeers.clear();
  if (initialized_) {
    enet_deinitialize();
    initialized_ = false;
  }
}

bool NetworkManager::startServer() {
  if (server_) {
    enet_host_destroy(server_);
    server_ = nullptr;
  }
  connectedPeers.clear();

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = kServerPort;

  server_ = enet_host_create(&address, kMaxClients, 2, 0, 0);
  if (!server_) {
    LOG_ERROR("Failed to create ENet server");
    return false;
  }
  LOG_INFO("ENet server started on port %d", kServerPort);
  return true;
}

bool NetworkManager::connectToServer(const std::string &address) {
  if (server_) {
    enet_host_destroy(server_);
    server_ = nullptr;
  }
  if (client_) {
    enet_host_destroy(client_);
    client_ = nullptr;
  }
  peer_ = nullptr;

  client_ = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client_) {
    LOG_ERROR("Failed to create ENet client");
    return false;
  }

  ENetAddress addr;
  enet_address_set_host(&addr, address.c_str());
  addr.port = kServerPort;

  peer_ = enet_host_connect(client_, &addr, 2, 0);
  if (!peer_) {
    LOG_ERROR("Failed to connect to server");
    enet_host_destroy(client_);
    client_ = nullptr;
    return false;
  }
  LOG_INFO("Connecting to server at %s:%d", address.c_str(), kServerPort);
  return true;
}

void NetworkManager::disconnect() {
  resetClientBombTracking();
  if (peer_) {
    enet_peer_disconnect(peer_, 0);
    peer_ = nullptr;
  }
  for (auto *p : connectedPeers) {
    enet_peer_disconnect(p, 0);
  }
  connectedPeers.clear();
}

void NetworkManager::resetClientBombTracking() {
  prevBombCount_ = 0;
  for (int i = 0; i < 32; i++) {
    prevBombPositions_[i] = {};
    prevBlastRadii_[i] = 0;
  }
}

bool NetworkManager::isConnected() {
  if (peer_) {
    return peer_->state == ENET_PEER_STATE_CONNECTED;
  }
  return false;
}

int NetworkManager::getNumConnectedPeers() {
  return static_cast<int>(connectedPeers.size());
}

void NetworkManager::sendPacket(void *data, size_t size, ENetPeer *target,
                                 uint8_t channel, bool reliable) {
  if (!target) return;
  enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED;
  ENetPacket *packet = enet_packet_create(data, size, flags);
  enet_peer_send(target, channel, packet);
  ENetHost *host = server_ ? server_ : client_;
  if (host) enet_host_flush(host);
}

bool NetworkManager::receivePacketFromHost(ENetHost *host, uint8_t *&outData,
                                            size_t &outLength,
                                            ENetPeer *&outSender) {
  ENetEvent event;
  while (enet_host_service(host, &event, 1) > 0) {
    LOG_DEBUG("receivePacketFromHost: got event type=%d",
              static_cast<int>(event.type));
    if (event.type == ENET_EVENT_TYPE_RECEIVE) {
      outLength = event.packet->dataLength;
      outSender = event.peer;
      static uint8_t pktBuf[2048];
      if (outLength <= sizeof(pktBuf)) {
        std::memcpy(pktBuf, event.packet->data, outLength);
        outData = pktBuf;
      } else {
        outData = nullptr;
        outLength = 0;
      }
      enet_packet_destroy(event.packet);
      LOG_DEBUG("receivePacketFromHost: RECEIVE len=%zu", outLength);
      return true;
    }
    if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
      LOG_INFO("receivePacketFromHost: DISCONNECT");
      if (server_) {
        for (auto it = connectedPeers.begin(); it != connectedPeers.end(); ++it) {
          if (*it == event.peer) { connectedPeers.erase(it); break; }
        }
      }
      if (client_ && event.peer == peer_) { peer_ = nullptr; }
    }
  }
  return false;
}

void NetworkManager::pollEvents() {
  if (server_) {
    ENetEvent event;
    while (enet_host_service(server_, &event, 0) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        LOG_INFO("Client connected from %x:%u", event.peer->address.host,
                 event.peer->address.port);
        connectedPeers.push_back(event.peer);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        LOG_INFO("Client disconnected");
        for (auto it = connectedPeers.begin(); it != connectedPeers.end();
             ++it) {
          if (*it == event.peer) {
            connectedPeers.erase(it);
            break;
          }
        }
        break;
      default:
        break;
      }
    }
  }

  if (client_) {
    ENetEvent event;
    while (enet_host_service(client_, &event, 0) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        LOG_INFO("Connected to server");
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        LOG_INFO("Disconnected from server");
        peer_ = nullptr;
        break;
      default:
        break;
      }
    }
  }
}

void NetworkManager::sendPlayerInput(int8_t direction, uint8_t plantBomb) {
  if (!client_ || !peer_) return;
  PlayerInputPacket payload;
  PacketHeader header{PacketType::PlayerInput};
  uint8_t buf[sizeof(PacketHeader) + sizeof(PlayerInputPacket)];
  std::memcpy(buf, &header, sizeof(header));
  payload.direction = direction;
  payload.plantBomb = plantBomb;
  std::memcpy(buf + sizeof(header), &payload, sizeof(payload));
  sendPacket(buf, sizeof(buf), peer_, 0, true);
}

void NetworkManager::sendCharSelect(uint8_t playerId, uint8_t charIndex,
                                     int peerIndex) {
  ENetPeer *target = nullptr;
  if (server_) {
    if (peerIndex >= 0 && peerIndex < static_cast<int>(connectedPeers.size())) {
      target = connectedPeers[peerIndex];
      LOG_INFO("sendCharSelect: sending to connectedPeer[%d] = %p, size=%zu",
               peerIndex, (void*)target, connectedPeers.size());
    } else if (!connectedPeers.empty()) {
      target = connectedPeers[0];
    }
    if (!target) {
      LOG_INFO("sendCharSelect: no target peer!");
      return;
    }
  } else if (client_) {
    target = peer_;
  }
  if (!target) return;
  CharSelectPacket payload;
  PacketHeader header{PacketType::CharSelect};
  uint8_t buf[sizeof(PacketHeader) + sizeof(CharSelectPacket)];
  std::memcpy(buf, &header, sizeof(header));
  payload.playerId = playerId;
  payload.charIndex = charIndex;
  std::memcpy(buf + sizeof(header), &payload, sizeof(payload));
  sendPacket(buf, sizeof(buf), target, 0, true);
}

void NetworkManager::sendAssignment(uint8_t clientPlayerId,
                                     uint8_t numHumanPlayers,
                                     const uint8_t charIds[4],
                                     int peerIndex) {
  if (!server_) return;
  AssignmentPacket payload;
  PacketHeader header{PacketType::Assignment};
  uint8_t buf[sizeof(PacketHeader) + sizeof(AssignmentPacket)];
  std::memcpy(buf, &header, sizeof(header));
  payload.clientPlayerId = clientPlayerId;
  payload.numHumanPlayers = numHumanPlayers;
  std::memcpy(payload.playerCharIds, charIds, 4);
  std::memcpy(buf + sizeof(header), &payload, sizeof(payload));

  if (peerIndex >= 0 && peerIndex < static_cast<int>(connectedPeers.size())) {
    sendPacket(buf, sizeof(buf), connectedPeers[peerIndex], 0, true);
  } else {
    for (auto *peer : connectedPeers) {
      sendPacket(buf, sizeof(buf), peer, 0, true);
    }
  }
}

void NetworkManager::broadcastGameState(const Game *game) {
  if (!server_) return;
  if (connectedPeers.empty()) return;

  GameStatePacket payload{};
  PacketHeader header{PacketType::GameState};
  uint8_t buf[sizeof(PacketHeader) + sizeof(GameStatePacket)];
  std::memcpy(buf, &header, sizeof(header));

  payload.numPlayers = 4;
  int bombIdx = 0;

  for (int i = 0; i < 4; i++) {
    const Player *p = game->playerManager.getPlayer(i);
    auto &pd = payload.players[i];
    pd.id = p->getId();
    pd.posX = p->getPosition().x;
    pd.posY = p->getPosition().y;
    pd.targetX = p->getTargetPosition().x;
    pd.targetY = p->getTargetPosition().y;
    pd.progress = p->getProgress();
    pd.speed = p->getSpeed();
    pd.facing = static_cast<uint8_t>(p->getFacing());
    pd.isAlive = p->isAlive() ? 1 : 0;
    pd.state = static_cast<uint8_t>(p->getState());
    pd.bombs = p->getBombs();
    pd.blastRadius = p->getBlastRadius();
  }

  for (auto &bomb : game->bombManager.getBombs()) {
    if (bomb != nullptr && bombIdx < kMaxBombStates) {
      auto &bd = payload.bombs[bombIdx];
      bd.posX = bomb->getPosition().x;
      bd.posY = bomb->getPosition().y;
      bd.remainingTime = bomb->getEndTime() - GetTime();
      bd.blastRadius = bomb->getBlastRadius();
      bd.ownerId = bomb->getOwnerId();
      bd.active = bomb->getEndTime() != 0 ? 1 : 0;
      bombIdx++;
    }
  }
  payload.numBombs = bombIdx;

  for (int x = 0; x < 15; x++) {
    for (int y = 0; y < 15; y++) {
      payload.grid[y * 15 + x] =
          static_cast<uint8_t>(game->grid.getCell({x, y}).type);
    }
  }

  std::memcpy(buf + sizeof(header), &payload, sizeof(payload));
  for (auto *peer : connectedPeers) {
    sendPacket(buf, sizeof(buf), peer, 1, false);
  }
}

bool NetworkManager::receivePlayerInput(int &outPlayerId,
                                         int &outDirection,
                                         uint8_t &outPlantBomb) {
  if (!server_) return false;
  uint8_t *data;
  size_t len;
  ENetPeer *sender;
  if (!receivePacketFromHost(server_, data, len, sender)) return false;
  if (len < sizeof(PacketHeader)) return false;
  PacketHeader *hdr = reinterpret_cast<PacketHeader *>(data);
  if (hdr->type != PacketType::PlayerInput) return false;
  if (len < sizeof(PacketHeader) + sizeof(PlayerInputPacket)) return false;
  auto *pkt = reinterpret_cast<PlayerInputPacket *>(data + sizeof(PacketHeader));
  outDirection = pkt->direction;
  outPlantBomb = pkt->plantBomb;

  outPlayerId = -1;
  for (size_t i = 0; i < connectedPeers.size(); i++) {
    if (connectedPeers[i] == sender) {
      outPlayerId = static_cast<int>(i + 1);
      break;
    }
  }
  return outPlayerId >= 0;
}

bool NetworkManager::receiveCharSelect(uint8_t &outPlayerId,
                                        uint8_t &outCharIndex) {
  ENetHost *host = server_ ? server_ : client_;
  if (!host) {
    LOG_INFO("receiveCharSelect: host is null");
    return false;
  }

  uint8_t *data;
  size_t len;
  ENetPeer *sender;
  if (!receivePacketFromHost(host, data, len, sender)) {
    LOG_DEBUG("receiveCharSelect: no packet from host");
    return false;
  }
  if (len < sizeof(PacketHeader)) {
    LOG_DEBUG("receiveCharSelect: packet too small for header");
    return false;
  }
  PacketHeader *hdr = reinterpret_cast<PacketHeader *>(data);
  if (hdr->type != PacketType::CharSelect) return false;
  if (len < sizeof(PacketHeader) + sizeof(CharSelectPacket)) return false;
  auto *pkt = reinterpret_cast<CharSelectPacket *>(data + sizeof(PacketHeader));
  outPlayerId = pkt->playerId;
  outCharIndex = pkt->charIndex;
  return true;
}

bool NetworkManager::receiveAssignment(uint8_t &outClientPlayerId,
                                        uint8_t &outNumHumanPlayers,
                                        uint8_t outCharIds[4]) {
  if (!client_) return false;
  uint8_t *data;
  size_t len;
  ENetPeer *sender;
  if (!receivePacketFromHost(client_, data, len, sender)) return false;
  if (len < sizeof(PacketHeader)) return false;
  PacketHeader *hdr = reinterpret_cast<PacketHeader *>(data);
  if (hdr->type != PacketType::Assignment) return false;
  if (len < sizeof(PacketHeader) + sizeof(AssignmentPacket)) return false;
  auto *pkt = reinterpret_cast<AssignmentPacket *>(data + sizeof(PacketHeader));
  outClientPlayerId = pkt->clientPlayerId;
  outNumHumanPlayers = pkt->numHumanPlayers;
  std::memcpy(outCharIds, pkt->playerCharIds, 4);
  return true;
}

bool NetworkManager::receiveGameState(Game *game) {
  if (!client_) return false;
  uint8_t *data;
  size_t len;
  ENetPeer *sender;
  while (receivePacketFromHost(client_, data, len, sender)) {
    if (len < sizeof(PacketHeader)) continue;
    PacketHeader *hdr = reinterpret_cast<PacketHeader *>(data);
    if (hdr->type != PacketType::GameState) continue;
    if (len < sizeof(PacketHeader) + sizeof(GameStatePacket)) continue;
    auto *pkt = reinterpret_cast<GameStatePacket *>(data + sizeof(PacketHeader));

    for (int i = 0; i < 4; i++) {
      Player *p = game->playerManager.getPlayer(i);
      auto &pd = pkt->players[i];
      p->setPosition({pd.posX, pd.posY});
      p->setTargetPosition({pd.targetX, pd.targetY});
      p->setProgress(pd.progress);
      p->setSpeed(pd.speed);
      p->setFacing(static_cast<Direction>(pd.facing));
      p->setState(static_cast<PlayerState>(pd.state));
      p->setBombs(pd.bombs);
      p->setBlastRadius(pd.blastRadius);
      if (!pd.isAlive) {
        p->kill();
      }
    }

    for (int x = 0; x < 15; x++) {
      for (int y = 0; y < 15; y++) {
        CellType type = static_cast<CellType>(pkt->grid[y * 15 + x]);
        game->grid.setCell({x, y}, type);
      }
    }

    {
      Position newPositions[32];
      int newBlastRadii[32];
      int newCount = 0;
      for (int i = 0; i < pkt->numBombs && i < 32; i++) {
        auto &bd = pkt->bombs[i];
        if (bd.active) {
          newPositions[newCount] = {bd.posX, bd.posY};
          newBlastRadii[newCount] = bd.blastRadius;
          newCount++;
        }
      }

      for (int i = 0; i < prevBombCount_; i++) {
        bool stillExists = false;
        for (int j = 0; j < newCount; j++) {
          if (prevBombPositions_[i].x == newPositions[j].x &&
              prevBombPositions_[i].y == newPositions[j].y) {
            stillExists = true;
            break;
          }
        }
        if (!stillExists) {
          game->bombManager.startLocalExplosion(prevBombPositions_[i],
                                                 prevBlastRadii_[i],
                                                 game->grid);
        }
      }

      std::memcpy(prevBombPositions_, newPositions, sizeof(Position) * newCount);
      std::memcpy(prevBlastRadii_, newBlastRadii, sizeof(int) * newCount);
      prevBombCount_ = newCount;

      Position positions[32];
      int ownerIds[32], blastRadii[32];
      double remainingTimes[32];
      int count = 0;
      for (int i = 0; i < pkt->numBombs && i < 32; i++) {
        auto &bd = pkt->bombs[i];
        if (bd.active) {
          positions[count] = {bd.posX, bd.posY};
          ownerIds[count] = bd.ownerId;
          blastRadii[count] = bd.blastRadius;
          remainingTimes[count] = bd.remainingTime;
          count++;
        }
      }
      game->bombManager.syncBombs(positions, ownerIds, blastRadii,
                                   remainingTimes, count);
    }
    return true;
  }
  return false;
}
