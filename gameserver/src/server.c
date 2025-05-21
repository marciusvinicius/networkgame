#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <enet/enet.h>

// Global variables
ENetHost *server;
ENetAddress address;
ENetEvent event;

Tile game_map[VIEWPORT_WIDTH][VIEWPORT_HEIGHT];
// extern global player_map, defined in main.c
extern ServerPlayerMap player_map;

// Initialize a new player
void init_player(Player *player, int id)
{
  // Define starting positions for players
  // These positions are spaced out in a grid pattern
  const int start_positions[8][2] = {
      {5, 5},   // Position 1
      {5, 8},   // Position 2
      {5, 10},  // Position 3
      {15, 10}, // Position 4
      {7, 7},   // Position 5
      {13, 7},  // Position 6
      {7, 13},  // Position 7
      {13, 13}  // Position 8
  };

  // Use the player ID to determine the starting position
  int position_index = id % 8;
  player->x = start_positions[position_index][0];
  player->y = start_positions[position_index][1];

  player->id = id;
  player->active = true;
  player->color_index = id % 8; // Assign color based on player ID (8 colors available)

  printf("Initialized player %d at position (%d, %d) with color %d\n", id,
         player->x, player->y, player->color_index);
}

// Remove a player from the game
void remove_player(ServerPlayerMap *map, ENetPeer *peer)
{
  for (int i = 0; i < map->count; i++)
  {
    if (map->entries[i].peer == peer)
    {
      map->entries[i].player.active = false;
      map->entries[i].player.x = 0;
      map->entries[i].player.y = 0;
      map->entries[i].player.id = -1; // Reset player ID
      PlayerIdPacket pkt;
      pkt.type = PKT_REMOVE_PLAYER;
      pkt.player_id = map->entries[i].player.id;
      ENetPacket *epkt = enet_packet_create(
          &pkt, sizeof(pkt), ENET_PACKET_FLAG_RELIABLE);
      enet_host_broadcast(server, 0, epkt);
      // Remove entry from map
      for (int j = i; j < map->count - 1; j++)
      {
        map->entries[j] = map->entries[j + 1];
      }
      map->count--;
      break;
    }
  }
}

// Get a player by their ENetPeer*
Player *get_player(ServerPlayerMap *map, ENetPeer *peer)
{
  printf("Looking up player for peer\n");
  for (int i = 0; i < map->count; i++)
  {
    if (map->entries[i].peer == peer && map->entries[i].player.active)
    {
      printf("Found player %d for peer\n", map->entries[i].player.id);
      return &map->entries[i].player;
    }
  }
  printf("No player found for peer\n");
  return NULL;
}

// Broadcast player positions to all connected clients
void broadcast_player_positions(ServerPlayerMap *map)
{
  PlayerPositionsPacket pkt;
  pkt.type = PKT_PLAYER_POSITIONS;
  pkt.player_count = 0;
  // Fill the packet with active player positions
  for (int i = 0; i < map->count; i++)
  {
    if (map->entries[i].player.active)
    {
      pkt.players[pkt.player_count].id = map->entries[i].player.id;
      pkt.players[pkt.player_count].x = map->entries[i].player.x;
      pkt.players[pkt.player_count].y = map->entries[i].player.y;
      pkt.player_count++;

      printf("Player %d position: (%d, %d) color: %d\n", map->entries[i].player.id,
             map->entries[i].player.x, map->entries[i].player.y, map->entries[i].player.color_index);

      printf("Broadcasting player positions to all clients. Player count: %d\n",
             pkt.player_count);
    }
  }

  // Create the packet for all player
  ENetPacket *epkt = enet_packet_create(
      &pkt, sizeof(unsigned char) * 2 + sizeof(struct {
                                          unsigned char id, x, y;
                                        }) * pkt.player_count,
      ENET_PACKET_FLAG_RELIABLE);
  // Broadcast to all connected peers
  enet_host_broadcast(server, 1, epkt);
  printf("Player positions broadcast complete\n");
}

// Load map from a text file
bool load_map_from_file(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    printf("Failed to open map file: %s\n", filename);
    return false;
  }

  char line[256];
  int y = 0;

  // Read the file line by line
  while (fgets(line, sizeof(line), file) && y < VIEWPORT_HEIGHT)
  {
    int x = 0;
    char *token = strtok(line, " \t\n");

    // Parse each number in the line
    while (token && x < VIEWPORT_WIDTH)
    {
      int tile_value = atoi(token);
      game_map[x][y].tile_id = tile_value;
      game_map[x][y].walkable =
          (tile_value == 1); // Only tiles with value 1 are walkable

      // Debug output for the first few tiles
      if (x < 3 && y < 3)
      {
        printf("Loaded tile at (%d, %d): id=%d, walkable=%d\n", x, y,
               game_map[x][y].tile_id, game_map[x][y].walkable);
      }
      x++;
      token = strtok(NULL, " \t\n");
    }

    y++;
  }

  fclose(file);
  printf("Map loaded successfully from %s\n", filename);

  // Print a summary of the map
  int walkable_count = 0;
  int total_tiles = VIEWPORT_WIDTH * VIEWPORT_HEIGHT;

  for (int y = 0; y < VIEWPORT_HEIGHT; y++)
  {
    for (int x = 0; x < VIEWPORT_WIDTH; x++)
    {
      if (game_map[x][y].walkable)
      {
        walkable_count++;
      }
    }
  }

  printf("Map summary: %d/%d tiles are walkable (%.1f%%)\n", walkable_count,
         total_tiles, (float)walkable_count / total_tiles * 100);

  return true;
}

// Initialize the server
void init_server(ServerPlayerMap *map)
{
  if (enet_initialize() != 0)
  {
    printf("Failed to initialize ENet\n");
    exit(EXIT_FAILURE);
  }

  // Create the server
  address.host = ENET_HOST_ANY;
  address.port = 8081;
  server = enet_host_create(&address, 32, 2, 0, 0);

  if (server == NULL)
  {
    printf("Failed to create ENet server\n");
    exit(EXIT_FAILURE);
  }

  printf("Server created successfully\n");

  // Initialize player map
  map->count = 0;
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    map->entries[i].player.active = false;
    map->entries[i].player.id = -1;
  }
}

// Broadcast game state to all connected clients
void broadcast_game_state(void)
{
  for (int i = 0; i < player_map.count; i++)
  {
    if (player_map.entries[i].player.active)
    {
      // Send tile chunks to the player
      send_tile_chunk(player_map.entries[i].peer, player_map.entries[i].player.x, player_map.entries[i].player.y);
    }
  }
}

// Process server events
void process_events(void)
{
  while (enet_host_service(server, &event, 0) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      handle_client_connection(&event);
      break;
    case ENET_EVENT_TYPE_DISCONNECT:
      handle_client_disconnect(&event);
      break;
    case ENET_EVENT_TYPE_RECEIVE:
      handle_client_packet(&event);
      break;
    default:
      break;
    }
  }
}

// Broadcast old players to a new player
void broadcast_old_players(ENetPeer *new_player)
{
  printf("Broadcasting old players to new player\n");
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (player_map.entries[i].player.active)
    {
      PlayerIdPacket add_packet = {
          PKT_ADD_PLAYER,
          player_map.entries[i].player.id,
          player_map.entries[i].player.color_index,
      };
      enet_peer_send(new_player, 0, enet_packet_create(&add_packet, sizeof(add_packet), ENET_PACKET_FLAG_RELIABLE));
      printf("Sent PKT_ADD_PLAYER for player %d to new player\n", player_map.entries[i].player.id);
    }
  }
}

// Handle client connection
void handle_client_connection(ENetEvent *event)
{
  printf("New client connected\n");

  // Find an available player slot
  int player_id = -1;
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (!player_map.entries[i].player.active)
    {
      player_id = i;
      break;
    }
  }

  if (player_id == -1)
  {
    printf("No available player slots\n");
    enet_peer_disconnect(event->peer, 0);
    return;
  }

  // Initialize the player
  player_map.entries[player_id].player.active = true;
  player_map.entries[player_id].peer = event->peer;
  init_player(&player_map.entries[player_id].player, player_id);

  // Send the player their ID
  PlayerIdPacket id_pkt;
  id_pkt.type = PKT_PLAYER_ID;
  id_pkt.player_id = player_id;
  id_pkt.color_index = player_map.entries[player_id].player.color_index;
  ENetPacket *epkt = enet_packet_create(&id_pkt, sizeof(id_pkt), ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(event->peer, 0, epkt);
  printf("Sent player ID %d to new client\n", player_id);

  // Send the entire map to the new player
  printf("Sending entire map to new client\n");
  for (int y = 0; y < VIEWPORT_HEIGHT / CHUNK_SIZE; y++)
  {
    for (int x = 0; x < VIEWPORT_WIDTH / CHUNK_SIZE; x++)
    {
      send_tile_chunk(event->peer, x, y);
      printf("Sent tile chunk (%d, %d) to new client\n", x, y);
    }
  }

  // Broadcast the new player to all other players
  PlayerIdPacket add_pkt;
  add_pkt.type = PKT_ADD_PLAYER;
  add_pkt.player_id = player_id;
  add_pkt.color_index = player_map.entries[player_id].player.color_index;
  epkt = enet_packet_create(&add_pkt, sizeof(add_pkt), ENET_PACKET_FLAG_RELIABLE);
  enet_host_broadcast(server, 0, epkt);
  printf("Broadcasted new player %d to all clients\n", player_id);

  // Broadcast old players to the new player
  printf("Broadcasting old players to new client\n");
  broadcast_old_players(event->peer);
  // Update player count
  player_map.count++;
}

// Send a tile chunk to a client
void send_tile_chunk(ENetPeer *peer, int chunk_x, int chunk_y)
{
  TileChunkPacket pkt;
  pkt.type = PKT_TILE_CHUNK;
  pkt.chunk_x = chunk_x;
  pkt.chunk_y = chunk_y;

  // Copy tiles from the game map to the packet
  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      int tile_x = chunk_x * CHUNK_SIZE + x;
      int tile_y = chunk_y * CHUNK_SIZE + y;
      if (tile_x >= 0 && tile_x < VIEWPORT_WIDTH && tile_y >= 0 && tile_y < VIEWPORT_HEIGHT)
      {
        pkt.tiles[y * CHUNK_SIZE + x] = game_map[tile_x][tile_y];
      }
      else
      {
        pkt.tiles[y * CHUNK_SIZE + x].tile_id = 0;
        pkt.tiles[y * CHUNK_SIZE + x].walkable = false;
      }
    }
  }

  ENetPacket *epkt = enet_packet_create(&pkt, sizeof(pkt), ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, epkt);
}

// Handle client disconnect
void handle_client_disconnect(ENetEvent *event)
{
  printf("Client disconnected\n");
  remove_player(&player_map, event->peer);
}

// Handle client packet
void handle_client_packet(ENetEvent *event)
{
  unsigned char *data = event->packet->data;
  unsigned char type = data[0];

  printf("Received packet type: %d\n", type);

  switch (type)
  {
  case PKT_MOVE:
    printf("Processing PKT_MOVE packet\n");
    process_move(event->peer, (MovePacket *)data);
    break;
  default:
    printf("Unknown packet type: %d\n", type);
    break;
  }

  enet_packet_destroy(event->packet);
}

// Process a move packet from a client
void process_move(ENetPeer *peer, MovePacket *pkt)
{
  printf("Processing move packet from client\n");
  printf("Move packet contents - dir_x: %d, dir_y: %d\n", pkt->dir_x, pkt->dir_y);

  Player *player = get_player(&player_map, peer);
  if (!player)
  {
    printf("Move from unknown player\n");
    return;
  }

  printf("Player %d current position: (%d, %d)\n", player->id, player->x, player->y);

  // Calculate new position based on current position and direction
  int new_x = player->x + pkt->dir_x;
  int new_y = player->y + pkt->dir_y;
  printf("Attempting to move to: (%d, %d)\n", new_x, new_y);

  // Check bounds
  if (new_x < 0 || new_x >= VIEWPORT_WIDTH || new_y < 0 || new_y >= VIEWPORT_HEIGHT)
  {
    printf("Move out of bounds: (%d, %d) - Rejecting movement\n", new_x, new_y);
    // Send current position back to client to correct invalid movement
    PlayerPositionsPacket pos_pkt;
    pos_pkt.type = PKT_PLAYER_POSITIONS;
    pos_pkt.player_count = 1;
    pos_pkt.players[player->id].id = player->id;
    pos_pkt.players[player->id].x = player->x;
    pos_pkt.players[player->id].y = player->y;
    ENetPacket *epkt = enet_packet_create(&pos_pkt, sizeof(pos_pkt), ENET_PACKET_FLAG_RELIABLE);
    if (enet_peer_send(peer, 0, epkt) < 0)
    {
      printf("Failed to send position correction packet\n");
    }
    else
    {
      printf("Sent position correction packet to player %d\n", player->id);
    }
    return;
  }

  // Check if the new position is walkable
  if (!game_map[new_x][new_y].walkable)
  {
    printf("Move to non-walkable tile: (%d, %d) - Rejecting movement\n", new_x, new_y);
    // Send current position back to client to correct invalid movement
    PlayerPositionsPacket pos_pkt;
    pos_pkt.type = PKT_PLAYER_POSITIONS;
    pos_pkt.player_count = 1;
    pos_pkt.players[player->id].id = player->id;
    pos_pkt.players[player->id].x = player->x;
    pos_pkt.players[player->id].y = player->y;
    ENetPacket *epkt = enet_packet_create(&pos_pkt, sizeof(pos_pkt), ENET_PACKET_FLAG_RELIABLE);
    if (enet_peer_send(peer, 0, epkt) < 0)
    {
      printf("Failed to send position correction packet\n");
    }
    else
    {
      printf("Sent position correction packet to player %d\n", player->id);
    }
    return;
  }

  // Update player position in the player map
  for (int i = 0; i < player_map.count; i++)
  {
    if (player_map.entries[i].player.id == player->id)
    {
      player_map.entries[i].player.x = new_x;
      player_map.entries[i].player.y = new_y;
      printf("Updated player %d position in map to (%d, %d)\n",
             player->id, new_x, new_y);
      break;
    }
  }

  // Also update the player pointer for consistency
  player->x = new_x;
  player->y = new_y;
  printf("Validated and applied movement for player %d to (%d, %d)\n",
         player->id, new_x, new_y);

  // Broadcast the new position to all clients
  broadcast_player_positions(&player_map);
}

// Cleanup server resources
void cleanup_server(void)
{
  enet_host_destroy(server);
  enet_deinitialize();
}
