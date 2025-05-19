#include "server.h"

// Global variables
ENetHost *server;
ENetAddress address;
ENetEvent event;

Tile game_map[VIEWPORT_WIDTH][VIEWPORT_HEIGHT];
Player players[MAX_PLAYERS];

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
  player->color_index =
      id % 8; // Assign color based on player ID (8 colors available)

  printf("Initialized player %d at position (%d, %d) with color %d\n", id,
         player->x, player->y, player->color_index);
  // Set the peer pointer to NULL initially
  player->peer = NULL;
  // Set the player to active
}

// Remove a player from the game
void remove_player(int id)
{
  if (id >= 0 && id < MAX_PLAYERS)
  {
    players[id].active = false;
    players[id].x = 0;
    players[id].y = 0;
    players[id].id = -1; // Reset player ID
  }
}

// Get a player by their ID
Player *get_player(int id)
{
  if (id >= 0 && id < MAX_PLAYERS && players[id].active)
  {
    return &players[id];
  }
  return NULL;
}

// Broadcast player positions to all connected clients
void broadcast_player_positions(void)
{
  PlayerPositionsPacket pkt;
  pkt.type = PKT_PLAYER_POSITIONS;
  pkt.player_count = 0;
  // Fill the packet with active player positions
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (players[i].active)
    {
      pkt.players[i].x = players[i].x;
      pkt.players[i].y = players[i].y;
      pkt.player_count = pkt.player_count + 1;

      printf("Player %d position: (%d, %d) color: %d\n", players[i].id,
             players[i].x, players[i].y, players[i].color_index);

      printf("Broadcasting player positions to all clients. Player count: %d\n",
             pkt.player_count);
    }
  }

  // Create the packet for all player
  ENetPacket *epkt = enet_packet_create(
      &pkt, sizeof(unsigned char) * 2 + sizeof(struct {
                                          unsigned char id, x, y, color_index;
                                        }) * pkt.player_count,
      ENET_PACKET_FLAG_RELIABLE);
  // Broadcast to all connected peers
  enet_host_broadcast(server, 0, epkt);
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
void init_server(void)
{
  if (enet_initialize() != 0)
  {
    printf("Failed to initialize ENet\n");
    exit(EXIT_FAILURE);
  }

  // Create the server
  address.host = ENET_HOST_ANY;
  address.port = 1234;
  server = enet_host_create(&address, 32, 2, 0, 0);

  if (server == NULL)
  {
    printf("Failed to create server\n");
    enet_deinitialize();
    exit(EXIT_FAILURE);
  }

  printf("Server initialized on port %d\n", address.port);

  // Load the map from file
  if (!load_map_from_file("map.txt"))
  {
    printf("Failed to load map, using default map\n");
    // Initialize the game map with default tiles if file loading fails
    for (int y = 0; y < VIEWPORT_HEIGHT; y++)
    {
      for (int x = 0; x < VIEWPORT_WIDTH; x++)
      {
        game_map[x][y].tile_id = 1;  // Default tile
        game_map[x][y].walkable = 1; // Default walkable
      }
    }
  }
}
//TODO: is not sending information about other
void broadcast_game_state(void)
{

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (players[i].active)
    {
      // Update player positions or other game logic here
      // For example, you can call a function to update player positions
      // based on input or other game events.
      send_tile_chunk(players[i].peer, players[i].x, players[i].y);
    }
  }
}

// Run the server main loop
void process_events(void)
{

  // Check for incoming connections
  // Wait for events
  // Wait for events
  while (enet_host_service(server, &event, 100) > 0)
  {
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
      handle_client_connection(&event);
      break;

    case ENET_EVENT_TYPE_RECEIVE:
      handle_client_packet(&event);
      break;

    case ENET_EVENT_TYPE_DISCONNECT:
      handle_client_disconnect(&event);
      break;
    }
  }
}

void broadcast_old_players(const int new_player_id) {
	//TODO:(marcius) Change it to send only on packet 
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
		if (players[i].id == new_player_id) continue;
        if (players[i].active)	{
          PlayerIdPacket add_pkt;
          add_pkt.type = PKT_ADD_PLAYER;
          add_pkt.player_id = players[i].id;
          add_pkt.color_index = players[i].color_index;
          ENetPacket *add_epkt =
              enet_packet_create(&add_pkt, sizeof(add_pkt), ENET_PACKET_FLAG_RELIABLE);
          enet_peer_send(players[new_player_id].peer, 0, add_epkt);
        }
    }
}

// Handle a new client connection
void handle_client_connection(ENetEvent *event)
{
  printf("Client connected from %x:%u\n", event->peer->address.host,
         event->peer->address.port);

  // Find an available player slot
  int player_id = -1;
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (!players[i].active)
    {
      player_id = i;
      players[i].active = true;
      break;
    }
  }

  if (player_id == -1)
  {
    printf("No available player slots!\n");
    enet_peer_disconnect(event->peer, 0);
    return;
  }

  // Initialize the new player
  init_player(&players[player_id], player_id);
  players[player_id].peer = event->peer;
  // Store the player ID in the peer's data
  event->peer->data = (void *)(intptr_t)player_id;

  // Send the player ID to the client
  PlayerIdPacket id_pkt;
  id_pkt.type = PKT_PLAYER_ID;
  id_pkt.player_id = player_id;
  id_pkt.color_index = players[player_id].color_index;
  ENetPacket *id_epkt =
      enet_packet_create(&id_pkt, sizeof(id_pkt), ENET_PACKET_FLAG_RELIABLE);

  // Broadcast to all connected peers
  enet_peer_send(event->peer, 0, id_epkt);
  //printf("Sent player ID %d to new client\n", player_id);
  //Send this new player to all players   
  PlayerIdPacket add_pkt;
  add_pkt.type = PKT_ADD_PLAYER;
  add_pkt.player_id = player_id;
  add_pkt.color_index = players[player_id].color_index;

  ENetPacket *add_epkt =
      enet_packet_create(&add_pkt, sizeof(add_pkt), ENET_PACKET_FLAG_RELIABLE);
  enet_host_broadcast(server, 0, add_epkt);
  broadcast_old_players(player_id);
  broadcast_game_state();
}



void send_tile_chunk(ENetPeer *peer, int chunk_x, int chunk_y)
{
  TileChunkPacket pkt;
  pkt.type = PKT_TILE_CHUNK;
  pkt.chunk_x = chunk_x;
  pkt.chunk_y = chunk_y;

  // Calculate the starting position of the chunk in the game map
  int start_x = chunk_x * CHUNK_SIZE;
  int start_y = chunk_y * CHUNK_SIZE;

  // Copy tiles from the game map to the packet
  int i = 0;
  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      // Check if the tile is within the game map bounds
      if (start_x + x < VIEWPORT_WIDTH && start_y + y < VIEWPORT_HEIGHT)
      {
        pkt.tiles[i] = game_map[start_x + x][start_y + y];
      }
      else
      {
        // If the tile is out of bounds, set it to a default value
        pkt.tiles[i].tile_id = 0;  // Empty/void tile
        pkt.tiles[i].walkable = 0; // Not walkable
      }
      i++;
    }
  }
}

// Handle a client disconnection
void handle_client_disconnect(ENetEvent *event)
{
  int player_id = (int)(intptr_t)event->peer->data;
  printf("Client %d disconnected\n", player_id);

  // Remove the player
  remove_player(player_id);

  // Broadcast updated player positions
  broadcast_player_positions();
}

// Handle a packet from a client
void handle_client_packet(ENetEvent *event)
{
  //printf("Received packet of size %d\n", event->packet->dataLength);

  // Check packet type
  if (event->packet->dataLength >= sizeof(MovePacket))
  {
    MovePacket *pkt = (MovePacket *)event->packet->data;

    if (pkt->type == PKT_MOVE)
    {
      printf("Received move packet: dir_x=%d, dir_y=%d\n", pkt->dir_x,
             pkt->dir_y);
      process_move(event->peer, pkt);
    }
  }
  // Clean up the packet
  enet_packet_destroy(event->packet);
}

// Process a move packet from a client
void process_move(ENetPeer *peer, MovePacket *pkt)
{
  int player_id = (int)(intptr_t)peer->data;
  Player *player = get_player(player_id);

  if (!player)
  {
    printf("Invalid player ID: %d\n", player_id);
    return;
  }

  printf("Processing move for player %d: dir_x=%d, dir_y=%d\n", player_id,
         pkt->dir_x, pkt->dir_y);

  // Calculate new position
  int new_x = player->x + pkt->dir_x;
  int new_y = player->y + pkt->dir_y;

  // Check if the new position is within bounds
  if (new_x < 0 || new_x >= VIEWPORT_WIDTH || new_y < 0 ||
      new_y >= VIEWPORT_HEIGHT)
  {
    printf("Player %d move rejected: new position (%d, %d) is out of bounds\n",
           player_id, new_x, new_y);
    return;
  }

  // Check if the new position is walkable
  if (!game_map[new_x][new_y].walkable)
  {
    printf("Player %d move rejected: new position (%d, %d) is not walkable "
           "(tile_id=%d)\n",
           player_id, new_x, new_y, game_map[new_x][new_y].tile_id);
    return;
  }

  // Calculate the old and new chunk positions
  int old_chunk_x = player->x / CHUNK_SIZE;
  int old_chunk_y = player->y / CHUNK_SIZE;
  int new_chunk_x = new_x / CHUNK_SIZE;
  int new_chunk_y = new_y / CHUNK_SIZE;

  // Update player position
  player->x = new_x;
  player->y = new_y;

  printf("Player %d moved to position (%d, %d)\n", player_id, player->x,
         player->y);

  // Check if the player moved to a new chunk
  if (old_chunk_x != new_chunk_x || old_chunk_y != new_chunk_y)
  {
    printf("Player %d moved to a new chunk: (%d, %d) -> (%d, %d)\n", player_id,
           old_chunk_x, old_chunk_y, new_chunk_x, new_chunk_y);

    // Send the new chunk to the player
    send_tile_chunk(peer, new_chunk_x, new_chunk_y);

    // Send adjacent chunks if needed
    if (new_x % CHUNK_SIZE < CHUNK_SIZE / 2)
    {
      // Player is in the left half of the chunk, send the chunk to the left
      if (new_chunk_x > 0)
      {
        send_tile_chunk(peer, new_chunk_x - 1, new_chunk_y);
      }
    }
    else
    {
      // Player is in the right half of the chunk, send the chunk to the right
      if (new_chunk_x < VIEWPORT_WIDTH / CHUNK_SIZE - 1)
      {
        send_tile_chunk(peer, new_chunk_x + 1, new_chunk_y);
      }
    }

    if (new_y % CHUNK_SIZE < CHUNK_SIZE / 2)
    {
      // Player is in the top half of the chunk, send the chunk above
      if (new_chunk_y > 0)
      {
        send_tile_chunk(peer, new_chunk_x, new_chunk_y - 1);
      }
    }
    else
    {
      // Player is in the bottom half of the chunk, send the chunk below
      if (new_chunk_y < VIEWPORT_HEIGHT / CHUNK_SIZE - 1)
      {
        send_tile_chunk(peer, new_chunk_x, new_chunk_y + 1);
      }
    }

    // Send diagonal chunks if needed
    if (new_x % CHUNK_SIZE < CHUNK_SIZE / 2 &&
        new_y % CHUNK_SIZE < CHUNK_SIZE / 2)
    {
      // Player is in the top-left quadrant, send the diagonal chunk
      if (new_chunk_x > 0 && new_chunk_y > 0)
      {
        send_tile_chunk(peer, new_chunk_x - 1, new_chunk_y - 1);
      }
    }
    else if (new_x % CHUNK_SIZE >= CHUNK_SIZE / 2 &&
             new_y % CHUNK_SIZE < CHUNK_SIZE / 2)
    {
      // Player is in the top-right quadrant, send the diagonal chunk
      if (new_chunk_x < VIEWPORT_WIDTH / CHUNK_SIZE - 1 && new_chunk_y > 0)
      {
        send_tile_chunk(peer, new_chunk_x + 1, new_chunk_y - 1);
      }
    }
    else if (new_x % CHUNK_SIZE < CHUNK_SIZE / 2 &&
             new_y % CHUNK_SIZE >= CHUNK_SIZE / 2)
    {
      // Player is in the bottom-left quadrant, send the diagonal chunk
      if (new_chunk_x > 0 && new_chunk_y < VIEWPORT_HEIGHT / CHUNK_SIZE - 1)
      {
        send_tile_chunk(peer, new_chunk_x - 1, new_chunk_y + 1);
      }
    }
    else if (new_x % CHUNK_SIZE >= CHUNK_SIZE / 2 &&
             new_y % CHUNK_SIZE >= CHUNK_SIZE / 2)
    {
      // Player is in the bottom-right quadrant, send the diagonal chunk
      if (new_chunk_x < VIEWPORT_WIDTH / CHUNK_SIZE - 1 &&
          new_chunk_y < VIEWPORT_HEIGHT / CHUNK_SIZE - 1)
      {
        send_tile_chunk(peer, new_chunk_x + 1, new_chunk_y + 1);
      }
    }
  }
  if (new_chunk_x != old_chunk_x || new_chunk_y != old_chunk_y)
  {
    printf("Player %d moved to a new chunk: (%d, %d) -> (%d, %d)\n", player_id,
           old_chunk_x, old_chunk_y, new_chunk_x, new_chunk_y);
    // TODO: Check if its the best approach to send the chunk to the player
    send_tile_chunk(peer, new_chunk_x, new_chunk_y);
  }
  // Broadcast updated player positions to all clients
  broadcast_player_positions();
}

// Clean up server resources
void cleanup_server(void)
{
  enet_host_destroy(server);
  enet_deinitialize();
  printf("Server shutdown complete\n");
}
