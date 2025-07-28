#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <netinet/in.h>

/**
 * Socket utility function prototypes
 */

/**
 * Create and configure a TCP server socket
 * @param port Port number to bind to
 * @return Socket file descriptor on success, -1 on error
 */
int create_server_socket(int port);

/**
 * Set socket to be reusable (SO_REUSEADDR)
 * @param socket_fd Socket file descriptor
 * @return 0 on success, -1 on error
 */
int set_socket_reusable(int socket_fd);

/**
 * Configure server address structure
 * @param addr Pointer to sockaddr_in structure to configure
 * @param port Port number
 */
void setup_server_address(struct sockaddr_in *addr, int port);

/**
 * Print error message with system error description
 * @param message Custom error message
 */
void print_error(const char *message);

/**
 * Print information message with timestamp
 * @param message Information message to print
 */
void print_info(const char *message);

/**
 * Convert socket address to readable string
 * @param addr Socket address structure
 * @param buffer Buffer to store the string
 * @param buffer_size Size of the buffer
 */
void addr_to_string(struct sockaddr_in *addr, char *buffer, size_t buffer_size);

#endif // SOCKET_UTILS_H