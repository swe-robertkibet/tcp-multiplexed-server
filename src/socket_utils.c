#define _POSIX_C_SOURCE 200112L
#include "../include/socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

/**
 * Create and configure a TCP server socket
 */
int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_error("Failed to create socket");
        return -1;
    }
    
    // Set socket to be reusable
    if (set_socket_reusable(server_fd) == -1) {
        close(server_fd);
        return -1;
    }
    
    // Setup server address
    setup_server_address(&server_addr, port);
    
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_error("Failed to bind socket");
        close(server_fd);
        return -1;
    }
    
    // Start listening for connections
    if (listen(server_fd, 10) == -1) {
        print_error("Failed to listen on socket");
        close(server_fd);
        return -1;
    }
    
    return server_fd;
}

/**
 * Set socket to be reusable (SO_REUSEADDR)
 */
int set_socket_reusable(int socket_fd) {
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        print_error("Failed to set socket options");
        return -1;
    }
    return 0;
}

/**
 * Configure server address structure
 */
void setup_server_address(struct sockaddr_in *addr, int port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(port);
}

/**
 * Print error message with system error description
 */
void print_error(const char *message) {
    fprintf(stderr, "[ERROR] %s: %s\n", message, strerror(errno));
}

/**
 * Print information message with timestamp
 */
void print_info(const char *message) {
    time_t now;
    char time_str[26];
    
    time(&now);
    ctime_r(&now, time_str);
    // Remove newline from time string
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[INFO] [%s] %s\n", time_str, message);
    fflush(stdout);
}

/**
 * Convert socket address to readable string
 */
void addr_to_string(struct sockaddr_in *addr, char *buffer, size_t buffer_size) {
    char ip_str[INET_ADDRSTRLEN];
    
    if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN) == NULL) {
        snprintf(buffer, buffer_size, "unknown");
        return;
    }
    
    snprintf(buffer, buffer_size, "%s:%d", ip_str, ntohs(addr->sin_port));
}