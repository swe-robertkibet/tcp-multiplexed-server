#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define DEFAULT_HOST "127.0.0.1"

/**
 * Print error message with system error description
 */
void print_client_error(const char *message) {
    fprintf(stderr, "[ERROR] %s: %s\n", message, strerror(errno));
}

/**
 * Print information message
 */
void print_client_info(const char *message) {
    printf("[INFO] %s\n", message);
    fflush(stdout);
}

/**
 * Create and connect to server socket
 */
int connect_to_server(const char *host, int port) {
    int client_fd;
    struct sockaddr_in server_addr;
    
    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        print_client_error("Failed to create socket");
        return -1;
    }
    
    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[ERROR] Invalid address: %s\n", host);
        close(client_fd);
        return -1;
    }
    
    // Connect to server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        print_client_error("Failed to connect to server");
        close(client_fd);
        return -1;
    }
    
    return client_fd;
}

/**
 * Send message to server and receive response
 */
int send_and_receive(int client_fd, const char *message) {
    char buffer[BUFFER_SIZE];
    int bytes_sent, bytes_received;
    
    // Send message to server
    bytes_sent = send(client_fd, message, strlen(message), 0);
    if (bytes_sent == -1) {
        print_client_error("Failed to send message");
        return -1;
    }
    
    printf("Sent: %s", message);
    
    // Receive response from server
    bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1) {
        print_client_error("Failed to receive response");
        return -1;
    } else if (bytes_received == 0) {
        print_client_info("Server closed the connection");
        return 0;
    }
    
    buffer[bytes_received] = '\0';
    printf("Received: %s", buffer);
    
    return bytes_received;
}

/**
 * Interactive mode - user can type messages
 */
void interactive_mode(int client_fd) {
    char input[BUFFER_SIZE];
    
    print_client_info("Connected to server. Type messages (or 'quit' to exit):");
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Check for quit command
        if (strncmp(input, "quit", 4) == 0) {
            break;
        }
        
        // Send message and receive response
        if (send_and_receive(client_fd, input) <= 0) {
            break;
        }
    }
}

/**
 * Automated test mode - send predefined messages
 */
void automated_test_mode(int client_fd) {
    const char *test_messages[] = {
        "Hello, Server!\n",
        "This is a test message\n", 
        "Testing TCP multiplexed server\n",
        "Message with numbers: 12345\n",
        "Special characters: !@#$%^&*()\n",
        NULL
    };
    
    int i = 0;
    
    print_client_info("Running automated tests...");
    
    while (test_messages[i] != NULL) {
        printf("\n--- Test %d ---\n", i + 1);
        
        if (send_and_receive(client_fd, test_messages[i]) <= 0) {
            break;
        }
        
        // Small delay between messages
        usleep(500000);  // 0.5 seconds
        i++;
    }
    
    print_client_info("Automated tests completed");
}

/**
 * Print usage information
 */
void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -h HOST      Server hostname/IP (default: %s)\n", DEFAULT_HOST);
    printf("  -p PORT      Server port (default: %d)\n", DEFAULT_PORT);
    printf("  -a           Run automated tests instead of interactive mode\n");
    printf("  -?           Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s                    # Connect to localhost:8080 (interactive)\n", program_name);
    printf("  %s -p 9090            # Connect to localhost:9090\n", program_name);
    printf("  %s -h 192.168.1.100   # Connect to specific IP\n", program_name);
    printf("  %s -a                 # Run automated tests\n", program_name);
}

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    int automated = 0;
    int client_fd;
    int opt;
    char connect_msg[256];
    
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "h:p:a?")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Error: Port must be between 1 and 65535\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'a':
                automated = 1;
                break;
            case '?':
            default:
                print_usage(argv[0]);
                return EXIT_SUCCESS;
        }
    }
    
    // Connect to server
    snprintf(connect_msg, sizeof(connect_msg), "Connecting to %s:%d...", host, port);
    print_client_info(connect_msg);
    
    client_fd = connect_to_server(host, port);
    if (client_fd == -1) {
        fprintf(stderr, "Failed to connect to server\n");
        return EXIT_FAILURE;
    }
    
    snprintf(connect_msg, sizeof(connect_msg), "Successfully connected to %s:%d", host, port);
    print_client_info(connect_msg);
    
    // Run in appropriate mode
    if (automated) {
        automated_test_mode(client_fd);
    } else {
        interactive_mode(client_fd);
    }
    
    // Clean up
    print_client_info("Closing connection...");
    close(client_fd);
    
    return EXIT_SUCCESS;
}