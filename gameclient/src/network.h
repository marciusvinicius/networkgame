#ifndef NETWORK_H
#define NETWORK_H

#include "game.h"

#include <stdbool.h>

// Define WIN32_LEAN_AND_MEAN before including Windows headers
#define WIN32_LEAN_AND_MEAN

#define ENET_IMPLEMENTATION

// ensure we are using winsock2 on windows.
#if (_WIN32_WINNT < 0x0601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

// Packet types
#define PKT_TILE_CHUNK 0x01
#define PKT_MOVE 0x02
#define PKT_PLAYER_POSITIONS 0x03
#define PKT_PLAYER_ID 0x04
#define PKT_ADD_PLAYER 0x05
// Define chunk size
#define CHUNK_SIZE 5
#define CHUNK_WIDTH 5
#define CHUNK_HEIGHT 5

// Function declarations
bool init_network(void);
bool connect_to_server(const char *host, int port);
void send_move(int dx, int dy);
void handle_network(void);
void disconnect(void);
bool is_connected(void);
void update_player_positions(const PlayerPositionsPacket *pkt);
void set_local_player_id(unsigned char player_id, unsigned char color_index);
int get_local_player_id();
void add_remote_player_id(unsigned char player_id, unsigned char color_index);
// Global variables
extern bool connected;
extern bool connection_confirmed;
extern int  connection_timeout;

#endif // NETWORK_H
