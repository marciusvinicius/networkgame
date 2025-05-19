// TODO:(marcius) Move this to a lib to be used by server and client, making
// easy to change structures that are common to both
#include <stdint.h>
#ifndef GAME_H
#define GAME_H

#define VIEWPORT_WIDTH 20
#define VIEWPORT_HEIGHT 15
#define MAX_PLAYERS 16
#define TILE_SIZE 32
#define GLOBAL static

typedef enum EntityType
{
  ENTITY_PLAYER,
  ENTITY_BULLET,
  ENTITY_ENEMY,
  ENTITY_ITEM,
} EntityType;

typedef struct
{
  unsigned char tile_id;
  unsigned char walkable;
} Tile;

// TODO: Network package should not know about this
typedef struct
{
  unsigned char type;
  unsigned char chunk_x;
  unsigned char chunk_y;
  Tile tiles[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
} TileChunkPacket;

// TODO: do I relly need a signed char? I can use 1 and 255
typedef struct
{
  unsigned char type;
  signed char dir_x;
  signed char dir_y;
} MovePacket;

// Player position packet structure
typedef struct
{
  unsigned char type;
  unsigned char player_count;
  struct
  {
    unsigned char id;
    signed char x;
    signed char y;
  } players[MAX_PLAYERS];
} PlayerPositionsPacket;

// Player ID packet structure
typedef struct
{
  unsigned char type;
  unsigned char player_id;
  unsigned char color_index;
} PlayerIdPacket;

#endif
