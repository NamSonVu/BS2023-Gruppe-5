#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "keyValStore.h"
#include "main.h"

int num_threads = NUM_THREADS;
pthread_t *threads;
pthread_t main_thread;

void* handle_client(void* arg) {
    int sockfd = *(int*) arg;
    char buf[BUF_SIZE];
    char key[BUF_SIZE];
    char response[BUF_SIZE];
    int nbytes;

    while ((nbytes = recv(sockfd, buf, BUF_SIZE, 0)) > 0) {
        buf[nbytes] = '\0';
        printf("Received message: %s\n", buf);


        // Parse the request
        char value[BUF_SIZE];
        if (sscanf(buf, "PUT %s %s", key, value) == 2) {
            if (put(key, value) == 0) {
                snprintf(response, BUF_SIZE, "PUT:%s:%s\n", key, value);
            } else {
                snprintf(response, BUF_SIZE, "PUT ERROR\n");
            }
        } else if (sscanf(buf, "GET %s", key) == 1) {
            if (get(key, value) == 0) {
                snprintf(response, BUF_SIZE, "GET:%s:%s\n", key, value);
            } else {
                snprintf(response, BUF_SIZE, "GET ERROR\n");
            }
        } else if (sscanf(buf, "DEL %s", key) == 1) {
            if (del(key) == 0) {
                snprintf(response, BUF_SIZE, "DEL:%s\n", key);
            } else {
                snprintf(response, BUF_SIZE, "DEL ERROR\n");
            }
        } else if (sscanf(buf, "QUIT") == 0) {
            printf("Received QUIT command, terminating server\n");
            // Cancel all threads
            pthread_cancel(main_thread);
            for (int i = 0; i < num_threads; i++) {
                pthread_cancel(threads[i]);
            }
            // Close the server socket and exit the process
            close(sockfd);
            exit(0);
        }
        else {
            snprintf(response, BUF_SIZE, "INVALID COMMAND\n");
        }

        // Send the response
        send(sockfd, response, strlen(response), 0);
        printf("Sent message: %s\n", response);
    }

    close(sockfd);
    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
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
    if (listen(sockfd, 5) != 0){
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

        // Create a new thread to handle the connection
        int* arg = malloc(sizeof(*arg));
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