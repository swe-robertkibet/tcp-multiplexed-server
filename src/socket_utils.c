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

// Global variable to track color support
static int colors_enabled = -1;  // -1 = not initialized, 0 = disabled, 1 = enabled

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
 * Check if terminal supports colors
 */
int terminal_supports_colors(void) {
    if (colors_enabled == -1) {
        // Check if stdout is a terminal and TERM is set
        if (isatty(STDOUT_FILENO)) {
            const char *term = getenv("TERM");
            if (term && strcmp(term, "dumb") != 0) {
                colors_enabled = 1;
            } else {
                colors_enabled = 0;
            }
        } else {
            colors_enabled = 0;
        }
    }
    return colors_enabled;
}

/**
 * Get color code for log type
 */
static const char* get_log_color(log_type_t type) {
    if (!terminal_supports_colors()) {
        return "";
    }
    
    switch (type) {
        case LOG_ERROR:      return COLOR_BRIGHT_RED;
        case LOG_SERVER:     return COLOR_BRIGHT_GREEN;
        case LOG_CONNECTION: return COLOR_BRIGHT_BLUE;
        case LOG_MESSAGE:    return COLOR_BRIGHT_YELLOW;
        case LOG_INFO:
        default:             return COLOR_GREEN;
    }
}

/**
 * Get log type prefix
 */
static const char* get_log_prefix(log_type_t type) {
    switch (type) {
        case LOG_ERROR:      return "ERROR";
        case LOG_SERVER:     return "SERVER";
        case LOG_CONNECTION: return "CONNECT";
        case LOG_MESSAGE:    return "MESSAGE";
        case LOG_INFO:
        default:             return "INFO";
    }
}

/**
 * Print error message with system error description
 */
void print_error(const char *message) {
    const char *color = terminal_supports_colors() ? COLOR_BRIGHT_RED : "";
    const char *reset = terminal_supports_colors() ? COLOR_RESET : "";
    fprintf(stderr, "%s[ERROR]%s %s: %s\n", color, reset, message, strerror(errno));
}

/**
 * Print colored log message with timestamp
 */
void print_log(log_type_t type, const char *message) {
    time_t now;
    char time_str[26];
    const char *color = get_log_color(type);
    const char *prefix = get_log_prefix(type);
    const char *reset = terminal_supports_colors() ? COLOR_RESET : "";
    const char *time_color = terminal_supports_colors() ? COLOR_CYAN : "";
    
    time(&now);
    ctime_r(&now, time_str);
    // Remove newline from time string
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("%s[%s]%s %s[%s]%s %s\n", color, prefix, reset, time_color, time_str, reset, message);
    fflush(stdout);
}

/**
 * Print information message with timestamp (legacy function)
 */
void print_info(const char *message) {
    print_log(LOG_INFO, message);
}

/**
 * Print server status message
 */
void print_server_info(const char *message) {
    print_log(LOG_SERVER, message);
}

/**
 * Print connection event message
 */
void print_connection_info(const char *message) {
    print_log(LOG_CONNECTION, message);
}

/**
 * Print message traffic information
 */
void print_message_info(const char *message) {
    print_log(LOG_MESSAGE, message);
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