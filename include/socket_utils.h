#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <netinet/in.h>

// ANSI Color Codes
#define COLOR_RESET     "\033[0m"
#define COLOR_RED       "\033[31m"
#define COLOR_GREEN     "\033[32m"
#define COLOR_YELLOW    "\033[33m"
#define COLOR_BLUE      "\033[34m"
#define COLOR_MAGENTA   "\033[35m"
#define COLOR_CYAN      "\033[36m"
#define COLOR_WHITE     "\033[37m"

// Bright color variants
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"

// Log message types
typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_CONNECTION,
    LOG_MESSAGE,
    LOG_SERVER
} log_type_t;

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
 * Check if terminal supports colors
 * @return 1 if colors are supported, 0 otherwise
 */
int terminal_supports_colors(void);

/**
 * Print error message with system error description
 * @param message Custom error message
 */
void print_error(const char *message);

/**
 * Print colored log message with timestamp
 * @param type Type of log message (determines color)
 * @param message Log message to print
 */
void print_log(log_type_t type, const char *message);

/**
 * Print information message with timestamp (legacy function)
 * @param message Information message to print
 */
void print_info(const char *message);

/**
 * Print server status message
 * @param message Server status message
 */
void print_server_info(const char *message);

/**
 * Print connection event message
 * @param message Connection event message
 */
void print_connection_info(const char *message);

/**
 * Print message traffic information
 * @param message Message traffic information
 */
void print_message_info(const char *message);

/**
 * Convert socket address to readable string
 * @param addr Socket address structure
 * @param buffer Buffer to store the string
 * @param buffer_size Size of the buffer
 */
void addr_to_string(struct sockaddr_in *addr, char *buffer, size_t buffer_size);

#endif // SOCKET_UTILS_H