#include "enet/enet.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "network.h"

// Global variables
ENetHost *client;
ENetAddress address;
ENetEvent event;
ENetPeer *peer;
bool connected = false;
bool connection_confirmed = false;
int connection_timeout = 60; // 60 frames timeout for connection

// External function to update player positions in the game
extern void update_player_positions(const PlayerPositionsPacket *pkt);
// External function to set the local player ID
extern void set_local_player_id(unsigned char new_player_id, unsigned char color_index);
extern int get_local_player_id();
// External variable for current tiles
extern Tile current_tiles[VIEWPORT_WIDTH][VIEWPORT_HEIGHT];

// Initialize network connection
bool init_network()
{
    if (enet_initialize() != 0)
    {
        printf("ENet initialization failed\n");
        return false;
    }
    client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL)
    {
        printf("Failed to create ENet host\n");
        enet_deinitialize();
        return false;
    }
    return true;
}

bool connect_to_server(const char *host, int port)
{
    address.port = port;
    enet_address_set_host(&address, host);
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL)
    {
        printf("Failed to connect to server\n");
        return false;
    }
    printf("Connecting to server...\n");
    connected = true;
    connection_confirmed = false;
    connection_timeout = 60; // Reset timeout
    return true;
}

void handle_network()
{
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf("Connected to server\n");
            connected = true;
            connection_confirmed = true;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            // Process the packet
            uint8_t packet_type = *(uint8_t *)event.packet->data;
            switch (packet_type)
            {
            case PKT_TILE_CHUNK:
            {
                TileChunkPacket *chunk = (TileChunkPacket *)event.packet->data;
                printf("Received tile chunk at (%d, %d)\n", chunk->chunk_x, chunk->chunk_y);
                // Calculate the starting position in the current_tiles array
                int start_x = chunk->chunk_x * CHUNK_SIZE;
                int start_y = chunk->chunk_y * CHUNK_SIZE;
                //TODO:(marcius) Move this to function
                // Copy the tiles from the packet to the current_tiles array
                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    for (int x = 0; x < CHUNK_SIZE; x++)
                    {
                        int tile_x = start_x + x;
                        int tile_y = start_y + y;

                        // Only copy if the tile is within the viewport bounds
                        if (tile_x >= 0 && tile_x < VIEWPORT_WIDTH && tile_y >= 0 && tile_y < VIEWPORT_HEIGHT)
                        {
                            current_tiles[tile_x][tile_y] = chunk->tiles[y * CHUNK_SIZE + x];
                        }
                    }
                }
                printf("Received tile chunk at (%d, %d)\n", chunk->chunk_x, chunk->chunk_y);
                break;
            }
            case PKT_PLAYER_POSITIONS:
            {
                printf("Received player positions packet\n");
                PlayerPositionsPacket *pos = (PlayerPositionsPacket *)event.packet->data;
                update_player_positions(pos);
                break;
            }
            case PKT_PLAYER_ID:
            {
                printf("Received player id\n");
                PlayerIdPacket *id_packet = (PlayerIdPacket *)event.packet->data;
                set_local_player_id(id_packet->player_id, id_packet->color_index);
                printf("Received player ID: %d, color: %d\n",
                       id_packet->player_id, id_packet->color_index);
                connection_confirmed = true;
                connected = true;
                break;
            }
            case PKT_ADD_PLAYER:
            {
                
                PlayerIdPacket *add_packet = (PlayerIdPacket *)event.packet->data;
                printf("Received player add %d \n", add_packet->player_id);
                printf("Local player id %d \n", get_local_player_id());
                if (add_packet->player_id == get_local_player_id()) break;
                add_remote_player_id(add_packet->player_id, add_packet->color_index);
                break;
            }
            case PKT_REMOVE_PLAYER:
            {
                PlayerIdPacket *remove_packet = (PlayerIdPacket *)event.packet->data;   
                printf("Received player remove %d \n", remove_packet->player_id);    
                printf("Local player id %d \n", get_local_player_id());
                if (remove_packet->player_id == get_local_player_id()) break;
                remove_remote_player_id(remove_packet->player_id);
                break;
            }
            //TODO: add pkt remove player / entity
            default:
                printf("Unknown packet type: %d\n", packet_type);
                enet_packet_destroy(event.packet);
                break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("Disconnected from server\n");
            connected = false;
            connection_confirmed = false;
            break;
        }
    }
}

void send_move(int dx, int dy)
{
    MovePacket pkt = {PKT_MOVE, dx, dy};
    ENetPacket *epkt = enet_packet_create(&pkt, sizeof(pkt), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, epkt);
}

void disconnect()
{
    enet_peer_disconnect(peer, 0);
    enet_host_destroy(client);
    enet_deinitialize();
}

// Check if the client is connected to the server
bool is_connected()
{
    return (peer != NULL && peer->state == ENET_PEER_STATE_CONNECTED);
}