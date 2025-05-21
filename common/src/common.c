#include "common.h"
#include <stdio.h>

// Initialize players array
void init_players(PlayerMap *map) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        map->players[i].active = false;
        map->players[i].x = 0;
        map->players[i].y = 0;
        map->players[i].id = -1;
        map->players[i].color_index = 0; // Default color index
    }
    map->count = 0;
}

// Update player positions based on server data
void update_player_positions(PlayerMap *map, const PlayerPositionsPacket *packet) {
    for (int i = 0; i < packet->player_count; i++) {
        int id = packet->players[i].id;
        for (int j = 0; j < map->count; j++) {
            if (map->players[id].id == id) {
                map->players[id].x = packet->players[id].x;
                map->players[id].y = packet->players[id].y;
                break;
            }
        }
    }
    printf("Updated positions - Active players: ");
    for (int i = 0; i < map->count; i++) {
        if (map->players[i].active) {
            printf("P%d(%d,%d) ", map->players[i].id, map->players[i].x, map->players[i].y);
        }
    }
    printf("\n");
}

// Add a remote player
void add_remote_player_id(PlayerMap *map, unsigned char player_id, unsigned char color_index) {
    int new_player_id = (int)player_id;
    for (int i = 0; i < map->count; i++) {
        if (map->players[i].id == new_player_id) {
            map->players[i].color_index = color_index;
            map->players[i].active = true;
            return;
        }
    }
    if (map->count < MAX_PLAYERS) {
        map->players[map->count].id = new_player_id;
        map->players[map->count].color_index = color_index;
        map->players[map->count].active = true;
        map->count++;
    }
}

// Remove a remote player
void remove_remote_player_id(PlayerMap *map, unsigned char player_id) {
    int new_player_id = (int)player_id;
    for (int i = 0; i < map->count; i++) {
        if (map->players[i].id == new_player_id) {
            map->players[i].active = false;
            map->players[i].id = -1;
            map->players[i].color_index = -1;
            map->players[i].x = 0;
            map->players[i].y = 0;
            break;
        }
    }
} 