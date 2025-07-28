#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>
#include <netinet/in.h>

#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

/**
 * Structure to track connected clients
 */
typedef struct {
    int socket_fd;                  // Client socket file descriptor
    struct sockaddr_in address;     // Client address information
    int active;                     // Whether this slot is active (1) or free (0)
} client_info_t;

/**
 * Server configuration and state
 */
typedef struct {
    int server_socket;              // Server socket file descriptor
    int port;                       // Server port number
    client_info_t clients[MAX_CLIENTS]; // Array of client connections
    fd_set master_set;              // Master file descriptor set
    fd_set read_set;                // Working file descriptor set for select()
    int max_fd;                     // Highest file descriptor number
    int running;                    // Server running flag
} server_t;

/**
 * Function prototypes
 */
int initialize_server(server_t *server, int port);
void run_server(server_t *server);
void shutdown_server(server_t *server);
void handle_new_connection(server_t *server);
void handle_client_message(server_t *server, int client_fd);
int find_client_index(server_t *server, int socket_fd);
void remove_client(server_t *server, int client_index);
void cleanup_server_resources(server_t *server);

#endif // SERVER_H