#include <stdio.h>
#include "keyValStore.h"
#include "main.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h>
#include <string.h>

#define MAX_TOKENS 3
#define MAX_TOKEN_LENGTH 50
#define VALUE_SIZE 64

#define PORT 5678
#define BUFFER_SIZE 1024

void error(const char *msg);

int tokenize(char *buffer, char **tokens) {
    int i = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL && i < MAX_TOKENS) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }

    if (i == MAX_TOKENS && strtok(NULL, " ") != NULL) {
        return -1;
    }

    for (int j = 0; j < i; j++) {
        if (strlen(tokens[j]) > MAX_TOKEN_LENGTH) {
            return -1;
        }
    }

    return i;
}

void handle_connection(int sockfd) {
    char buffer[BUFFER_SIZE];
    int n;

    // Keep reading commands from the client until the client quits
    while (1) {
        // Read a command from the client
        bzero(buffer, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE);
        if (n < 0) {
            error("Error reading from socket");
            return;
        }

        // Tokenize the command string
        char *tokens[3];
        tokenize(buffer, tokens);

        // Process the command
        if (strcmp(tokens[0], "GET") == 0) {
            char value[VALUE_SIZE];
            int result = get(tokens[1], value);
            if (result < 0) {
                write(sockfd, "GET:key:key_nonexistent", strlen("GET:key:key_nonexistent"));
            } else {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "GET:%s:%s", tokens[1], value);
                write(sockfd, response, strlen(response));
            }
        } else if (strcmp(tokens[0], "PUT") == 0) {
            int result = put(tokens[1], tokens[2]);
            if (result < 0) {
                error("Error putting key-value pair");
            } else {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "PUT:%s:%s", tokens[1], tokens[2]);
                write(sockfd, response, strlen(response));
            }
        } else if (strcmp(tokens[0], "DEL") == 0) {
            int result = del(tokens[1]);
            if (result < 0) {
                write(sockfd, "DEL:key:key_nonexistent", strlen("DEL:key:key_nonexistent"));
            } else {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "DEL:%s:key_deleted", tokens[1]);
                write(sockfd, response, strlen(response));
            }
        } else if (strcmp(tokens[0], "QUIT") == 0) {
            // Close the socket and return
            close(sockfd);
            return;
        } else {
            // Invalid command
            error("Invalid command");
        }
    }
}



int main() {

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int addrlen = sizeof(server_addr);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

// Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

// Set server address and bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

// Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept incoming connection
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Handle connection
        handle_connection(client_fd);

        // Close client connection
        close(client_fd);
    }




}


