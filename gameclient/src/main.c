#include <stdio.h>
#include <string.h>

#include "game.h"
#include "network.h"
#include "raylib.h"

// Global
Tile current_tiles[VIEWPORT_WIDTH][VIEWPORT_HEIGHT];
// Use the PlayerMap from common.h
PlayerMap player_map = {0};
int local_player_id = -1;

// Player colors based on server assignment
const Color PLAYER_COLORS[8] = {
    {255, 0, 0, 255},     // RED
    {0, 0, 255, 255},     // BLUE
    {0, 255, 0, 255},     // GREEN
    {255, 255, 0, 255},   // YELLOW
    {128, 0, 128, 255},   // PURPLE
    {255, 165, 0, 255},   // ORANGE
    {255, 192, 203, 255}, // PINK
    {135, 206, 235, 255}  // SKYBLUE
};

// Initialize players array
void init_players(PlayerMap *map) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    map->entries[i].player.active = false;
    map->entries[i].player.x = 0;
    map->entries[i].player.y = 0;
    map->entries[i].player.id = -1;
    map->entries[i].player.color_index = 0; // Default color index
  }
  map->count = 0;
}

// Update player positions based on server data
void update_player_positions(PlayerMap *map, const PlayerPositionsPacket *packet) {
  for (int i = 0; i < packet->player_count; i++) {
    int id = packet->players[i].id;
    for (int j = 0; j < map->count; j++) {
      if (map->entries[j].player.id == id) {
        map->entries[j].player.x = packet->players[i].x;
        map->entries[j].player.y = packet->players[i].y;
        break;
      }
    }
  }
  printf("Updated positions - Active players: ");
  for (int i = 0; i < map->count; i++) {
    if (map->entries[i].player.active) {
      printf("P%d(%d,%d) ", map->entries[i].player.id, map->entries[i].player.x, map->entries[i].player.y);
    }
  }
  printf("\n");
}

// Draw all active players
void draw_players(PlayerMap *map) {
  printf("Drawing players. Local player ID: %d\n", local_player_id);
  int active_count = 0;
  for (int i = 0; i < map->count; i++) {
    if (!map->entries[i].player.active) {
      continue;
    }
    active_count++;
    printf("Drawing player %d at position (%d, %d) with color %d\n",
           map->entries[i].player.id, map->entries[i].player.x, map->entries[i].player.y, map->entries[i].player.color_index);

    Color player_color = PLAYER_COLORS[map->entries[i].player.color_index % 8];
    int screen_x = map->entries[i].player.x * TILE_SIZE;
    int screen_y = map->entries[i].player.y * TILE_SIZE;

    DrawRectangle(screen_x + 2, screen_y + 2, TILE_SIZE, TILE_SIZE, (Color){0, 0, 0, 100});
    DrawRectangle(screen_x, screen_y, TILE_SIZE, TILE_SIZE, player_color);
    DrawRectangleLines(screen_x, screen_y, TILE_SIZE, TILE_SIZE, WHITE);

    char id_text[10];
    sprintf(id_text, "%d", map->entries[i].player.id);
    int text_width = MeasureText(id_text, 20);
    int text_x = screen_x + (TILE_SIZE - text_width) / 2;
    int text_y = screen_y + (TILE_SIZE - 20) / 2;
    DrawRectangle(text_x - 2, text_y - 2, text_width + 4, 24, (Color){0, 0, 0, 150});
    DrawText(id_text, text_x, text_y, 20, WHITE);

    if (map->entries[i].player.id == local_player_id) {
      const char *you_text = "YOU";
      int you_width = MeasureText(you_text, 15);
      int you_x = screen_x + (TILE_SIZE - you_width) / 2;
      int you_y = screen_y + TILE_SIZE + 5;
      DrawRectangle(you_x - 2, you_y - 2, you_width + 4, 19, (Color){0, 0, 0, 150});
      DrawText(you_text, you_x, you_y, 15, WHITE);
      int indicator_size = TILE_SIZE / 4;
      int indicator_x = screen_x + (TILE_SIZE - indicator_size) / 2;
      int indicator_y = screen_y + (TILE_SIZE - indicator_size) / 2;
      DrawRectangle(indicator_x, indicator_y, indicator_size, indicator_size, (Color){255, 255, 255, 200});
    } else {
      const char *other_text = "OTHER";
      int other_width = MeasureText(other_text, 15);
      int other_x = screen_x + (TILE_SIZE - other_width) / 2;
      int other_y = screen_y + TILE_SIZE + 5;
      DrawRectangle(other_x - 2, other_y - 2, other_width + 4, 19, (Color){0, 0, 0, 150});
      DrawText(other_text, other_x, other_y, 15, WHITE);
    }
  }
  printf("Drew %d active players\n", active_count);
}

// Handle a tile chunk from the server
void handle_tile_chunk(const TileChunkPacket *pkt) {
  printf("Received tile chunk at position (%d, %d)\n", pkt->chunk_x,
         pkt->chunk_y);

  // Calculate the starting position in the current_tiles array
  int start_x = pkt->chunk_x * CHUNK_SIZE;
  int start_y = pkt->chunk_y * CHUNK_SIZE;

  // Copy the tiles from the packet to the current_tiles array
  for (int y = 0; y < CHUNK_SIZE; y++) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
      int tile_x = start_x + x;
      int tile_y = start_y + y;

      // Only copy if the tile is within the viewport bounds
      if (tile_x >= 0 && tile_x < VIEWPORT_WIDTH && tile_y >= 0 &&
          tile_y < VIEWPORT_HEIGHT) {
        current_tiles[tile_x][tile_y] = pkt->tiles[y * CHUNK_SIZE + x];
      }
    }
  }

  printf("Updated %d tiles in chunk (%d, %d)\n", CHUNK_SIZE * CHUNK_SIZE,
         pkt->chunk_x, pkt->chunk_y);
}

void draw_tiles() {
  printf("Drawing tiles\n");

  int tile_count = 0;
  for (int y = 0; y < VIEWPORT_HEIGHT; y++) {
    for (int x = 0; x < VIEWPORT_WIDTH; x++) {
      tile_count++;
      Color col = GRAY; // Default color
      // Set color based on tile_id
      switch (current_tiles[x][y].tile_id) {
      case 0: // Empty/void
        col = BLACK;
        break;
      case 1: // Grass
        col = DARKGREEN;
        break;
      case 2: // Water
        col = BLUE;
        break;
      case 3: // Sand
        col = YELLOW;
        break;
      case 4: // Stone
        col = DARKGRAY;
        break;
      default:
        col = PURPLE; // Unknown tile type
        break;
      }
      printf("Drawing tile %d at position (%d, %d) with color %d\n", current_tiles[x][y].tile_id, x, y, col);
      printf("Tile ID: %d\n", current_tiles[x][y].tile_id);
      printf("Tile walkable: %d\n", current_tiles[x][y].walkable);

      // Draw the tile
      DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, col);

      // Draw a grid line
      DrawRectangleLines(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                         (Color){50, 50, 50, 100});
    }
  }

  printf("Drew %d tiles\n", tile_count);
}

int get_local_player_id() {
    return local_player_id;
}
// Set the local player ID and color
void set_local_player_id(PlayerMap *map, unsigned char new_player_id, unsigned char color_index) {
  printf("Setting local player ID to %d with color %d\n", new_player_id, color_index);
  local_player_id = (int)new_player_id;
  bool found = false;
  for (int i = 0; i < map->count; i++) {
    if (map->entries[i].player.id == local_player_id) {
      map->entries[i].player.color_index = (int)color_index;
      map->entries[i].player.active = true;
      found = true;
      break;
    }
  }
  if (!found && map->count < MAX_PLAYERS) {
    map->entries[map->count].player.id = local_player_id;
    map->entries[map->count].player.color_index = (int)color_index;
    map->entries[map->count].player.active = true;
    map->count++;
  }
}

// Initialize the game window
bool init_window() {
  InitWindow(VIEWPORT_WIDTH * TILE_SIZE, VIEWPORT_HEIGHT * TILE_SIZE,
             "Game Client");
  SetTargetFPS(60);
  return true;
}

int main(int argc, char *argv[]) {
  // Initialize network
  if (!init_network()) {
    printf("Failed to initialize network\n");
    return 1;
  }

  // Connect to server
  if (!connect_to_server("localhost", 8081)) {
    printf("Failed to connect to server\n");
    return 1;
  }

  // Initialize players
  init_players(&player_map);

  // Initialize tile array
  memset(current_tiles, 0, sizeof(current_tiles));

  // Initialize window
  if (!init_window()) {
    printf("Failed to initialize window\n");
    return 1;
  }

  // Main game loop
  while (1) {
    // Handle network messages
    handle_network();
    // TODO: simulate locally and then send a pull of commmands to the server
    // Consider delta time for movement
    //  Handle user input
    if (is_connected()) {
      if (IsKeyDown(KEY_UP)) {
        send_move(0, -1);
      }
      if (IsKeyDown(KEY_DOWN)) {
        send_move(0, 1);
      }
      if (IsKeyDown(KEY_LEFT)) {
        send_move(-1, 0);
      }
      if (IsKeyDown(KEY_RIGHT)) {
        send_move(1, 0);
      }
    }
    // Update connection status
    if (connected && !connection_confirmed) {
      connection_timeout--;
      if (connection_timeout <= 0) {
        connected = false;
        printf("Connection timeout\n");
      }
    }

    // Draw game
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw tiles
    draw_tiles();
    // Draw players
    draw_players(&player_map);
    // Draw connection status
    if (!is_connected()) {
      DrawText("Connecting to server...", 10, 10, 20, RED);
      DrawText("Press ESC to quit", 10, 40, 20, RED);
    }

    EndDrawing();

    // Check for window close
    if (WindowShouldClose()) {
      break;
    }
  }
  // Cleanup
  disconnect();
  CloseWindow();
  return 0;
}

// Add a remote player
void add_remote_player_id(PlayerMap *map, unsigned char player_id, unsigned char color_index) {
  int new_player_id = (int)player_id;
  if (new_player_id == local_player_id) return;

  // Check if the player is already in the map
  for (int i = 0; i < map->count; i++) {
    if (map->entries[i].player.id == new_player_id) {
      map->entries[i].player.color_index = color_index;
      map->entries[i].player.active = true;
      printf("Updated existing player %d in map\n", new_player_id);
      return;
    }
  }

  // If the player is not in the map, add them
  if (map->count < MAX_PLAYERS) {
    map->entries[map->count].player.id = new_player_id;
    map->entries[map->count].player.color_index = color_index;
    map->entries[map->count].player.active = true;
    map->entries[map->count].player.x = 0; // Default position
    map->entries[map->count].player.y = 0; // Default position
    map->count++;
    printf("Added new player %d to map\n", new_player_id);
  } else {
    printf("Cannot add player %d: map is full\n", new_player_id);
  }
}

// Remove a remote player
void remove_remote_player_id(PlayerMap *map, unsigned char player_id) {
  int new_player_id = (int)player_id;
  if (new_player_id == local_player_id) return;
  for (int i = 0; i < map->count; i++) {
    if (map->entries[i].player.id == new_player_id) {
      map->entries[i].player.active = false;
      map->entries[i].player.id = -1;
      map->entries[i].player.color_index = -1;
      map->entries[i].player.x = 0;
      map->entries[i].player.y = 0;
      break;
    }
  }
}