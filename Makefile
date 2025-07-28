# TCP Multiplexed Server Makefile
# Professional build configuration for university assignment

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O2 -DNDEBUG

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SERVER_SOURCES = $(SRC_DIR)/server.c $(SRC_DIR)/socket_utils.c $(SRC_DIR)/client_handler.c
CLIENT_SOURCES = $(SRC_DIR)/test_client.c

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target executables
SERVER_TARGET = $(BIN_DIR)/tcp_server
CLIENT_TARGET = $(BIN_DIR)/test_client

# Include path
INCLUDES = -I$(INCLUDE_DIR)

# Default target
.PHONY: all debug release clean install uninstall help test

all: debug

# Debug build (default)
debug: CFLAGS += $(DEBUG_FLAGS)
debug: directories $(SERVER_TARGET) $(CLIENT_TARGET)

# Release build (optimized)
release: CFLAGS += $(RELEASE_FLAGS)
release: directories $(SERVER_TARGET) $(CLIENT_TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Server executable
$(SERVER_TARGET): $(SERVER_OBJECTS)
	@echo "Linking server executable..."
	$(CC) $(SERVER_OBJECTS) -o $@
	@echo "Server built successfully: $@"

# Client executable  
$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	@echo "Linking client executable..."
	$(CC) $(CLIENT_OBJECTS) -o $@
	@echo "Client built successfully: $@"

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Dependencies (header files)
$(OBJ_DIR)/server.o: $(SRC_DIR)/server.c $(INCLUDE_DIR)/server.h $(INCLUDE_DIR)/socket_utils.h $(INCLUDE_DIR)/client_handler.h
$(OBJ_DIR)/socket_utils.o: $(SRC_DIR)/socket_utils.c $(INCLUDE_DIR)/socket_utils.h
$(OBJ_DIR)/client_handler.o: $(SRC_DIR)/client_handler.c $(INCLUDE_DIR)/client_handler.h $(INCLUDE_DIR)/server.h $(INCLUDE_DIR)/socket_utils.h
$(OBJ_DIR)/test_client.o: $(SRC_DIR)/test_client.c

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete."

# Install binaries (optional)
install: release
	@echo "Installing binaries..."
	sudo cp $(SERVER_TARGET) /usr/local/bin/
	sudo cp $(CLIENT_TARGET) /usr/local/bin/
	@echo "Installation complete."

# Uninstall binaries
uninstall:
	@echo "Uninstalling binaries..."
	sudo rm -f /usr/local/bin/tcp_server
	sudo rm -f /usr/local/bin/test_client
	@echo "Uninstallation complete."

# Run basic functionality test
test: debug
	@echo "Running basic server test..."
	@echo "Starting server in background..."
	./$(SERVER_TARGET) 8081 > server_test.log 2>&1 &
	@SERVER_PID=$$!; \
	sleep 2; \
	echo "Running automated client test..."; \
	./$(CLIENT_TARGET) -p 8081 -a || true; \
	echo "Stopping server..."; \
	kill $$SERVER_PID 2>/dev/null || true; \
	wait $$SERVER_PID 2>/dev/null || true; \
	echo "Test completed. Check server_test.log for server output."

# Display help information
help:
	@echo "TCP Multiplexed Server - Build System"
	@echo "===================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build debug version (default)"
	@echo "  debug     - Build debug version with debugging symbols"
	@echo "  release   - Build optimized release version"
	@echo "  clean     - Remove all build artifacts"
	@echo "  test      - Run basic functionality test"
	@echo "  install   - Install binaries to /usr/local/bin (requires sudo)"
	@echo "  uninstall - Remove installed binaries (requires sudo)"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build debug version"
	@echo "  make release      # Build optimized version"
	@echo "  make clean        # Clean build files"
	@echo "  make test         # Test the server"
	@echo ""
	@echo "Running the programs:"
	@echo "  ./$(SERVER_TARGET) [port]          # Start server (default port: 8080)"
	@echo "  ./$(CLIENT_TARGET) -h host -p port # Connect client to server"
	@echo "  ./$(CLIENT_TARGET) -a              # Run automated tests"

# Display project information
info:
	@echo "TCP Multiplexed Server Project Information"
	@echo "========================================="
	@echo "Compiler: $(CC)"
	@echo "Debug Flags: $(DEBUG_FLAGS)"
	@echo "Release Flags: $(RELEASE_FLAGS)"
	@echo "Source Directory: $(SRC_DIR)"
	@echo "Include Directory: $(INCLUDE_DIR)"
	@echo "Object Directory: $(OBJ_DIR)"
	@echo "Binary Directory: $(BIN_DIR)"
	@echo "Server Target: $(SERVER_TARGET)"
	@echo "Client Target: $(CLIENT_TARGET)"