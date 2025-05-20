#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <enet/enet.h>

// Common definitions
#define VIEWPORT_WIDTH 20
#define VIEWPORT_HEIGHT 15
#define MAX_PLAYERS 32
#define TILE_SIZE 32
#define CHUNK_SIZE 5

// Common packet types
#define PKT_TILE_CHUNK 0x01
#define PKT_MOVE 0x02
#define PKT_PLAYER_POSITIONS 0x03
#define PKT_PLAYER_ID 0x04
#define PKT_ADD_PLAYER 0x05
#define PKT_REMOVE_PLAYER 0x06

// Common structures
typedef struct {
  unsigned char tile_id;
  unsigned char walkable;
} Tile;

typedef struct {
  unsigned char type;
  unsigned char chunk_x;
  unsigned char chunk_y;
  Tile tiles[CHUNK_SIZE * CHUNK_SIZE];
} TileChunkPacket;

typedef struct {
  unsigned char type;
  signed char dir_x;
  signed char dir_y;
} MovePacket;

typedef struct {
  unsigned char type;
  unsigned char player_count;
  struct {
    unsigned char id;
    signed char x;
    signed char y;
  } players[MAX_PLAYERS];
} PlayerPositionsPacket;

typedef struct {
  unsigned char type;
  unsigned char player_id;
  unsigned char color_index;
} PlayerIdPacket;

// Base Player structure (common fields)
typedef struct {
  int x;
  int y;
  int id;
  unsigned char color_index;
  bool active;
} Player;

// Common structures
typedef struct {
  ENetPeer* peer;
  Player player;
} PeerPlayerEntry;

typedef struct {
  PeerPlayerEntry entries[MAX_PLAYERS];
  int count;
} PlayerMap;

// Common player management functions
void init_players(PlayerMap *map);
void update_player_positions(PlayerMap *map, const PlayerPositionsPacket *packet);
void add_remote_player_id(PlayerMap *map, unsigned char player_id, unsigned char color_index);
void remove_remote_player_id(PlayerMap *map, unsigned char player_id);

#endif // COMMON_H 