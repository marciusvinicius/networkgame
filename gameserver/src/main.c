#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "server.h"

// Global flag for graceful shutdown
volatile int running = 1;

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    printf("Received signal %d, shutting down...\n", signum);
    running = 0;
}

int main(void) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize the server
    init_server();
    
    // Run the server
    printf("Server started. Press Ctrl+C to stop.\n");
    
    // Main server loop
    while (running) {
        run_server();
    }
    
    // Clean up
    cleanup_server();
    
    return 0;
} 