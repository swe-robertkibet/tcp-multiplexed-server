#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "server.h"

/**
 * Client handling function prototypes
 */

/**
 * Initialize client information structure
 * @param client Pointer to client_info_t structure
 */
void init_client_info(client_info_t *client);

/**
 * Add a new client to the server's client list
 * @param server Pointer to server structure
 * @param client_fd Client socket file descriptor
 * @param client_addr Client address information
 * @return Index of the added client, -1 if server is full
 */
int add_client(server_t *server, int client_fd, struct sockaddr_in *client_addr);

/**
 * Read message from client socket
 * @param client_fd Client socket file descriptor
 * @param buffer Buffer to store the message
 * @param buffer_size Size of the buffer
 * @return Number of bytes read, 0 if client disconnected, -1 on error
 */
int read_client_message(int client_fd, char *buffer, size_t buffer_size);

/**
 * Send message to client socket
 * @param client_fd Client socket file descriptor
 * @param message Message to send
 * @param message_len Length of the message
 * @return Number of bytes sent, -1 on error
 */
int send_client_message(int client_fd, const char *message, size_t message_len);

/**
 * Process and echo client message
 * @param server Pointer to server structure
 * @param client_fd Client socket file descriptor
 * @param buffer Message buffer
 * @param bytes_received Number of bytes received
 */
void process_client_message(server_t *server, int client_fd, char *buffer, int bytes_received);

/**
 * Clean up client resources and remove from server
 * @param server Pointer to server structure
 * @param client_index Index of client to remove
 */
void cleanup_client(server_t *server, int client_index);

/**
 * Get count of active clients
 * @param server Pointer to server structure
 * @return Number of active clients
 */
int get_active_client_count(server_t *server);

#endif // CLIENT_HANDLER_H