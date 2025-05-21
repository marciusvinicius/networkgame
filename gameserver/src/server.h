#ifndef SERVER_H
#define SERVER_H

#include <enet/enet.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../common/src/common.h"

// Server-specific structures
typedef struct {
  Player base;  // Inherit from base Player struct
  ENetPeer *peer; // Pointer to the ENet peer
  bool is_local;
} ServerPlayer;

// Use the common PeerPlayerEntry
typedef struct {
  PeerPlayerEntry entries[MAX_PLAYERS];
  int count;
} ServerPlayerMap;

// Server-specific functions
void init_server(ServerPlayerMap *map);
void cleanup_server(void);
void process_events(void);
void broadcast_game_state(void);
void broadcast_player_positions(ServerPlayerMap *map);
void remove_player(ServerPlayerMap *map, ENetPeer* peer);
Player *get_player(ServerPlayerMap *map, ENetPeer* peer);
void handle_client_connection(ENetEvent *event);
void handle_client_disconnect(ENetEvent *event);
void handle_client_packet(ENetEvent *event);
void send_tile_chunk(ENetPeer *peer, int chunk_x, int chunk_y);
void process_move(ENetPeer *peer, MovePacket *pkt);
void broadcast_old_players(ENetPeer *new_player);
bool load_map_from_file(const char *filename);

#endif // SERVER_H
