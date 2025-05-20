#ifndef GAME_H
#define GAME_H

#include "../../common/src/common.h"

typedef enum EntityType {
  ENTITY_PLAYER,
  ENTITY_BULLET,
  ENTITY_ENEMY,
  ENTITY_ITEM,
} EntityType;

// Player management functions
void init_players(PlayerMap *map);
void update_player_positions(PlayerMap *map, const PlayerPositionsPacket *packet);
void draw_players(PlayerMap *map);
void set_local_player_id(PlayerMap *map, unsigned char new_player_id, unsigned char color_index);
void add_remote_player_id(PlayerMap *map, unsigned char player_id, unsigned char color_index);
void remove_remote_player_id(PlayerMap *map, unsigned char player_id);

#endif // GAME_H 