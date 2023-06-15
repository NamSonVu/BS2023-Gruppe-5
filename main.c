#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "keyValStore.h"
#include "main.h"
#include "sub.c"

int num_threads = NUM_THREADS;
pthread_t *threads;
pthread_t main_thread;
char client_names[MAX_CLIENTS] = {0};

void generate_client_name(char* client_name) {
    // Find the next available client name based on the incoming connection order
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!client_names[i]) {
            client_names[i] = 1;
            *client_name = 'A' + i;
            return;
        }
    }

    // If all client names are used, assign a default name
    strcpy(client_name, "Unknown");
}

void *handle_client(void *arg) {
    int sockfd = *(int *) arg;
    char buf[BUF_SIZE];
    char key[BUF_SIZE];
    char response[BUF_SIZE];
    int nbytes;

    // Get the client IP address
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    getpeername(sockfd, (struct sockaddr *) &addr, &addr_len);

    // Get the client name
    char client_name[BUF_SIZE];
    generate_client_name(client_name);

    while ((nbytes = recv(sockfd, buf, BUF_SIZE, 0)) > 0) {
        buf[nbytes] = '\0';
        printf("Received message from client %s: %s\n", client_name, buf);

        // Parse the request
        char value[BUF_SIZE];
        int command = -1; // Initialize to invalid value

        // Check if the command starts with "PUT"
        if (strncasecmp(buf, "PUT", 3) == 0) {
            // Try to parse the command
            if (sscanf(buf, "PUT %s %s", key, value) == 2) {
                command = 0; // PUT command
            }
        }
            // Check if the command starts with "GET"
        else if (strncasecmp(buf, "GET", 3) == 0) {
            // Try to parse the command
            if (sscanf(buf, "GET %s", key) == 1) {
                command = 1; // GET command
            }
        }
            // Check if the command starts with "DEL"
        else if (strncasecmp(buf, "DEL", 3) == 0) {
            // Try to parse the command
            if (sscanf(buf, "DEL %s", key) == 1) {
                command = 2; // DEL command
            }
        }
            // Check if the command is "QUIT"
        else if (strcasecmp(buf, "QUIT") == 0) {
            command = 3; // QUIT command
        }
        else if (strncasecmp(buf, "SUB", 3) == 0) {
            // Try to parse the command
            if (sscanf(buf, "SUB %s", key) == 1) {
                command = 4; // SUB command
            }
        }

        // Handle the command
        switch (command) {
            case 0: // PUT
                if (put(key, value, sockfd, client_name) == 0) {
                    snprintf(response, BUF_SIZE, "PUT:%s:%s\n", key, value);
                    //printf("Client %s created %s with value %s\n", client_name, key, value);
                } else {
                    snprintf(response, BUF_SIZE, "PUT ERROR\n");
                }
                break;

            case 1: // GET
                if (get(key, value, sockfd, client_name) == 0) {
                    snprintf(response, BUF_SIZE, "GET:%s:%s\n", key, value);
                    //printf("Client %s retrieved %s with value %s\n", client_name, key, value);
                } else {
                    snprintf(response, BUF_SIZE, "GET ERROR\n");
                }
                break;

            case 2: // DEL
                if (del(key, value, sockfd, client_name) == 0) {
                    snprintf(response, BUF_SIZE, "DEL:%s\n", key);
                    //printf("Client %s deleted %s\n", client_name, key, value);
                } else {
                    snprintf(response, BUF_SIZE, "DEL ERROR\n");
                }
                break;

            case 3: // QUIT
                printf("Received QUIT command from client %s, terminating server\n", client_name);
                // Cancel all threads
                pthread_cancel(main_thread);
                for (int i = 0; i < num_threads; i++) {
                    pthread_cancel(threads[i]);
                }
                // Close the server socket and exit the process
                close(sockfd);
                exit(0);
                break;

            case 4: // SUB
                subscribe(sockfd, key, client_name);
                snprintf(response, BUF_SIZE, "SUBSCRIBED:%s\n", key);
                //printf(response, BUF_SIZE, "Client %s subscribed to %s\n", client_name, key);
                break;

            default:
                snprintf(response, BUF_SIZE, "INVALID COMMAND\n");
                break;
        }

        // Send the response
        send(sockfd, response, strlen(response), 0);
        printf("Sent message to client %s: %s\n", client_name, response);
    }

    close(sockfd);
    free(arg);
    return NULL;
}



int main(int argc, char *argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, clientaddr;

    // Initialize the key-value store
    initKeyValStore();

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(sockfd, 5) != 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Key-value store server listening on port %d...\n", PORT);


    while (1) {
        // Accept a new connection
        socklen_t clientaddr_len = sizeof(clientaddr);
        connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &clientaddr_len);
        if (connfd == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Send a welcome message to the client
        char welcome_message[BUF_SIZE];
        snprintf(welcome_message, BUF_SIZE, "Welcome to the Key-Value Server!\n\n"
                                            "This server allows you to store and retrieve key-value pairs using the following functions:\n\n"
                                            "| Function | Description                        | Usage         |\n"
                                            "|----------|------------------------------------|---------------|\n"
                                            "| PUT      | Stores a key-value pair in database| PUT key val   |\n"
                                            "| GET      | Retrieves value for a given key    | GET key res   |\n"
                                            "| DEL      | Removes a key-value pair from db   | DEL key       |\n\n"
                                            "To use these functions in your code, simply call them with the appropriate arguments as shown.\n"
                                            "Thank you for using our Key-Value Server!\n");
        send(connfd, welcome_message, strlen(welcome_message), 0);
        // Create a new thread to handle the connection
        int *arg = malloc(sizeof(*arg));
        if (arg == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        *arg = connfd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, arg) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}