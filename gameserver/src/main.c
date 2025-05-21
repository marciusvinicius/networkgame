#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "server.h"

// Global variables
ServerPlayerMap player_map = {0};

// Global flag for graceful shutdown
// This flag is set to 0 when a signal is received, indicating that the server should stop running.
volatile int running = 1;

// Signal handler for graceful shutdown
void signal_handler(int signum)
{
    printf("\nReceived signal %d, shutting down...\n", signum);
    cleanup_server();
    exit(0);
}

int main(void)
{
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (!load_map_from_file("map.txt")) {
        exit(1);
    }
    // Initialize the server
    init_server(&player_map);

    // Run the server
    printf("Server started. Press Ctrl+C to stop.\n");

    float last_time = 0;
    // Main server loop

    printf("Server running...\n");
    while (running)
    {
        // Check for incoming events and process them
        process_events();
        //broadcast_game_state();
        // Broadcast updated player positions
        broadcast_player_positions(&player_map);
        //  add some sleep to avoid busy waiting
    }

    // Clean up
    cleanup_server();

    return 0;
}