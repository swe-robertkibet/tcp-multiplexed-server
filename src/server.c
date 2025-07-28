#include "../include/server.h"
#include "../include/socket_utils.h"
#include "../include/client_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>

// Global server instance for signal handling
static server_t *g_server = NULL;

/**
 * Signal handler for graceful shutdown
 */
void signal_handler(int sig) {
    if (g_server != NULL) {
        print_info("Received shutdown signal, stopping server...");
        g_server->running = 0;
    }
}

/**
 * Initialize server structure and create listening socket
 */
int initialize_server(server_t *server, int port) {
    int i;
    char info_msg[256];
    
    // Initialize server structure
    server->port = port;
    server->running = 1;
    server->max_fd = 0;
    
    // Initialize all client slots as inactive
    for (i = 0; i < MAX_CLIENTS; i++) {
        init_client_info(&server->clients[i]);
    }
    
    // Create server socket
    server->server_socket = create_server_socket(port);
    if (server->server_socket == -1) {
        return -1;
    }
    
    // Initialize file descriptor sets
    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_set);
    
    // Add server socket to master set
    FD_SET(server->server_socket, &server->master_set);
    server->max_fd = server->server_socket;
    
    snprintf(info_msg, sizeof(info_msg), "Server initialized on port %d", port);
    print_info(info_msg);
    
    return 0;
}

/**
 * Main server loop using select() for I/O multiplexing
 */
void run_server(server_t *server) {
    int i, activity;
    char info_msg[256];
    
    g_server = server;
    
    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    snprintf(info_msg, sizeof(info_msg), "Server listening on port %d", server->port);
    print_info(info_msg);
    print_info("Press Ctrl+C to stop the server");
    
    while (server->running) {
        // Copy master set to working set
        server->read_set = server->master_set;
        
        // Wait for activity on any socket
        activity = select(server->max_fd + 1, &server->read_set, NULL, NULL, NULL);
        
        if (activity < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, continue
                continue;
            }
            print_error("select() failed");
            break;
        }
        
        // Check if there's activity on the server socket (new connection)
        if (FD_ISSET(server->server_socket, &server->read_set)) {
            handle_new_connection(server);
        }
        
        // Check all client sockets for activity
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].active && 
                FD_ISSET(server->clients[i].socket_fd, &server->read_set)) {
                handle_client_message(server, server->clients[i].socket_fd);
            }
        }
    }
    
    print_info("Server shutting down...");
    shutdown_server(server);
}

/**
 * Handle new client connection
 */
void handle_new_connection(server_t *server) {
    int client_fd, client_index;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char addr_str[64];
    char info_msg[256];
    
    // Accept new connection
    client_fd = accept(server->server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        print_error("Failed to accept client connection");
        return;
    }
    
    // Add client to server's client list
    client_index = add_client(server, client_fd, &client_addr);
    if (client_index == -1) {
        // Server is full, reject connection
        addr_to_string(&client_addr, addr_str, sizeof(addr_str));
        snprintf(info_msg, sizeof(info_msg), "Server full, rejecting connection from %s", addr_str);
        print_info(info_msg);
        close(client_fd);
        return;
    }
    
    // Add client socket to master set
    FD_SET(client_fd, &server->master_set);
    if (client_fd > server->max_fd) {
        server->max_fd = client_fd;
    }
    
    // Log new connection
    addr_to_string(&client_addr, addr_str, sizeof(addr_str));
    snprintf(info_msg, sizeof(info_msg), "New client connected from %s (clients: %d/%d)", 
             addr_str, get_active_client_count(server), MAX_CLIENTS);
    print_info(info_msg);
}

/**
 * Handle message from existing client
 */
void handle_client_message(server_t *server, int client_fd) {
    char buffer[BUFFER_SIZE];
    int bytes_received, client_index;
    
    // Read message from client
    bytes_received = read_client_message(client_fd, buffer, sizeof(buffer));
    
    if (bytes_received <= 0) {
        // Client disconnected or error occurred
        client_index = find_client_index(server, client_fd);
        if (client_index != -1) {
            remove_client(server, client_index);
        }
        return;
    }
    
    // Process the received message
    process_client_message(server, client_fd, buffer, bytes_received);
}

/**
 * Find client index by socket file descriptor
 */
int find_client_index(server_t *server, int socket_fd) {
    int i;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].active && server->clients[i].socket_fd == socket_fd) {
            return i;
        }
    }
    
    return -1;
}

/**
 * Remove client from server and clean up resources
 */
void remove_client(server_t *server, int client_index) {
    char addr_str[64];
    char info_msg[256];
    int client_fd;
    
    if (client_index < 0 || client_index >= MAX_CLIENTS || 
        !server->clients[client_index].active) {
        return;
    }
    
    client_fd = server->clients[client_index].socket_fd;
    
    // Log client disconnection
    addr_to_string(&server->clients[client_index].address, addr_str, sizeof(addr_str));
    snprintf(info_msg, sizeof(info_msg), "Client %s disconnected (clients: %d/%d)", 
             addr_str, get_active_client_count(server) - 1, MAX_CLIENTS);
    print_info(info_msg);
    
    // Remove from file descriptor set
    FD_CLR(client_fd, &server->master_set);
    
    // Clean up client resources
    cleanup_client(server, client_index);
    
    // Update max_fd if necessary
    if (client_fd == server->max_fd) {
        int i;
        server->max_fd = server->server_socket;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].active && server->clients[i].socket_fd > server->max_fd) {
                server->max_fd = server->clients[i].socket_fd;
            }
        }
    }
}

/**
 * Shutdown server and clean up all resources
 */
void shutdown_server(server_t *server) {
    cleanup_server_resources(server);
    print_info("Server shutdown complete");
}

/**
 * Clean up all server resources
 */
void cleanup_server_resources(server_t *server) {
    int i;
    
    // Close all client connections
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].active) {
            cleanup_client(server, i);
        }
    }
    
    // Close server socket
    if (server->server_socket != -1) {
        close(server->server_socket);
        server->server_socket = -1;
    }
    
    // Clear file descriptor sets
    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_set);
}

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    server_t server;
    int port = DEFAULT_PORT;
    
    // Parse command line arguments
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            fprintf(stderr, "Port must be between 1 and 65535\n");
            return EXIT_FAILURE;
        }
    }
    
    // Initialize server
    if (initialize_server(&server, port) == -1) {
        fprintf(stderr, "Failed to initialize server\n");
        return EXIT_FAILURE;
    }
    
    // Run server
    run_server(&server);
    
    return EXIT_SUCCESS;
}