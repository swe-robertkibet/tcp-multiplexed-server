#include "../include/client_handler.h"
#include "../include/socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

/**
 * Initialize client information structure
 */
void init_client_info(client_info_t *client) {
    client->socket_fd = -1;
    client->active = 0;
    memset(&client->address, 0, sizeof(client->address));
}

/**
 * Add a new client to the server's client list
 */
int add_client(server_t *server, int client_fd, struct sockaddr_in *client_addr) {
    int i;
    
    // Find first available slot
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!server->clients[i].active) {
            server->clients[i].socket_fd = client_fd;
            server->clients[i].address = *client_addr;
            server->clients[i].active = 1;
            return i;
        }
    }
    
    // No available slots
    return -1;
}

/**
 * Read message from client socket
 */
int read_client_message(int client_fd, char *buffer, size_t buffer_size) {
    int bytes_received;
    
    bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the string
    } else if (bytes_received == 0) {
        // Client closed connection gracefully
        return 0;
    } else {
        // Error occurred
        if (errno != ECONNRESET) {
            print_error("Failed to receive data from client");
        }
        return -1;
    }
    
    return bytes_received;
}

/**
 * Send message to client socket
 */
int send_client_message(int client_fd, const char *message, size_t message_len) {
    int bytes_sent;
    
    bytes_sent = send(client_fd, message, message_len, 0);
    
    if (bytes_sent == -1) {
        if (errno != EPIPE && errno != ECONNRESET) {
            print_error("Failed to send data to client");
        }
        return -1;
    }
    
    return bytes_sent;
}

/**
 * Process and echo client message
 */
void process_client_message(server_t *server, int client_fd, char *buffer, int bytes_received) {
    char response[BUFFER_SIZE + 64];  // Extra space for response formatting
    char addr_str[64];
    char log_msg[512];
    int client_index;
    
    // Find client information for logging
    client_index = find_client_index(server, client_fd);
    if (client_index == -1) {
        return;  // Client not found
    }
    
    // Get client address string for logging
    addr_to_string(&server->clients[client_index].address, addr_str, sizeof(addr_str));
    
    // Remove trailing newline/carriage return from received message
    while (bytes_received > 0 && 
           (buffer[bytes_received - 1] == '\n' || buffer[bytes_received - 1] == '\r')) {
        buffer[bytes_received - 1] = '\0';
        bytes_received--;
    }
    
    // Log received message
    snprintf(log_msg, sizeof(log_msg), "Received from %s: \"%s\"", addr_str, buffer);
    print_message_info(log_msg);
    
    // Create echo response
    snprintf(response, sizeof(response), "Echo: %s\n", buffer);
    
    // Send echo response back to client
    if (send_client_message(client_fd, response, strlen(response)) == -1) {
        // Failed to send response, client likely disconnected
        remove_client(server, client_index);
        return;
    }
    
    // Log sent response
    snprintf(log_msg, sizeof(log_msg), "Sent to %s: \"Echo: %s\"", addr_str, buffer);
    print_info(log_msg);
}

/**
 * Clean up client resources and remove from server
 */
void cleanup_client(server_t *server, int client_index) {
    if (client_index < 0 || client_index >= MAX_CLIENTS) {
        return;
    }
    
    // Close client socket
    if (server->clients[client_index].socket_fd != -1) {
        close(server->clients[client_index].socket_fd);
    }
    
    // Reset client information
    init_client_info(&server->clients[client_index]);
}

/**
 * Get count of active clients
 */
int get_active_client_count(server_t *server) {
    int count = 0;
    int i;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].active) {
            count++;
        }
    }
    
    return count;
}