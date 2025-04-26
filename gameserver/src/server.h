#ifndef SERVER_H
#define SERVER_H

#include "enet/enet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define the same packet types as the client
#define PKT_TILE_CHUNK 0x01
#define PKT_MOVE 0x02
#define PKT_PLAYER_POSITIONS 0x03
#define PKT_PLAYER_ID 0x04 // New packet type for sending player ID

// Define the same viewport dimensions as the client
#define VIEWPORT_WIDTH 20
#define VIEWPORT_HEIGHT 15

// Define chunk size for tile loading
#define CHUNK_SIZE 5

// Define the same tile structure as the client
typedef struct
{
    unsigned char tile_id;
    unsigned char walkable;
} Tile;

// Define the same packet structures as the client
typedef struct
{
    unsigned char type;
    unsigned char chunk_x;
    unsigned char chunk_y;
    Tile tiles[CHUNK_SIZE * CHUNK_SIZE];
} TileChunkPacket;

typedef struct
{
    unsigned char type;
    unsigned char dir_x;
    unsigned char dir_y;
} MovePacket;

// Maximum number of players
#define MAX_PLAYERS 32

// Player position packet structure
typedef struct
{
    unsigned char type;
    unsigned char player_count;
    struct
    {
        unsigned char id;
        unsigned char x;
        unsigned char y;
        unsigned char color_index; // Added color information
        unsigned char is_active;   // Added active status
    } players[MAX_PLAYERS];
} PlayerPositionsPacket;

// Player ID packet structure
typedef struct
{
    unsigned char type;
    unsigned char player_id;
    unsigned char color_index;
} PlayerIdPacket;

// Player structure
typedef struct
{
    int x;
    int y;
    int id;                    // Connection ID (used for color assignment)
    unsigned char color_index; // Index into the color array
    bool active;
} Player;

// Player management functions
void init_player(Player *player, int id);
void remove_player(int id);
Player *get_player(int id);
void broadcast_player_positions(void);

// Server functions
void init_server(void);
void run_server(void);
void handle_client_connection(ENetEvent *event);
void handle_client_disconnect(ENetEvent *event);
void handle_client_packet(ENetEvent *event);
void send_tile_chunk(ENetPeer *peer, int chunk_x, int chunk_y);
void process_move(ENetPeer *peer, MovePacket *pkt);
void cleanup_server(void);
bool load_map_from_file(const char *filename);

// External variables
extern ENetHost *server;
extern ENetAddress address;
extern ENetEvent event;
extern Tile game_map[VIEWPORT_WIDTH][VIEWPORT_HEIGHT];

#endif // SERVER_H