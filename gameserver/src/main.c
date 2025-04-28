#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "server.h"
// Global flag for graceful shutdown
// This flag is set to 0 when a signal is received, indicating that the server should stop running.
volatile int running = 1;

// Signal handler for graceful shutdown
void signal_handler(int signum)
{
    printf("Received signal %d, shutting down...\n", signum);
    running = 0;
}

int main(void)
{
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize the server
    init_server();

    // Run the server
    printf("Server started. Press Ctrl+C to stop.\n");

    float last_time = 0;
    // Main server loop

    printf("Server running...\n");
    while (running)
    {
        // Check for incoming events and process them
        process_events();
        broadcast_game_state();
        // Broadcast updated player positions
        broadcast_player_positions();
        //  add some sleep to avoid busy waiting
    }

    // Clean up
    cleanup_server();

    return 0;
}