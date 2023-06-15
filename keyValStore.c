#include <string.h>
#include "keyValStore.h"
#include "main.h"
#include <stdio.h>
#include <sys/socket.h>


typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int used;
    int subscribers[MAX_SUBSCRIBERS];
    int num_subscribers;
} KeyValue;

static KeyValue keyvalstore[MAX_KEYS];
static Subscription subscriptions[MAX_SUBSCRIBERS];

void initKeyValStore() {
    memset(keyvalstore, 0, sizeof(KeyValue) * MAX_KEYS);
    memset(subscriptions, 0, sizeof(Subscription) * MAX_SUBSCRIBERS);
}

int put(char* key, char* value, int client_socket, char* client_name) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            strcpy(keyvalstore[i].value, value);
            printf("Client %s changed %s to %s\n", client_name, key, value);

            //to initialize the function
            put_successful(key, value);

            return 0;
        }
    }

    for (int i = 0; i < MAX_KEYS; i++) {
        if (!keyvalstore[i].used) {
            strcpy(keyvalstore[i].key, key);
            strcpy(keyvalstore[i].value, value);
            keyvalstore[i].used = 1;
            printf("Client %s creates %s with %s\n", client_name, key, value);
            return 0;
        }
    }

    return -1;
}

void put_successful(char* key, char* value) {
    // Iterate over the keys
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            // Iterate over the subscribers of the key
            for (int j = 0; j < keyvalstore[i].num_subscribers; j++) {
                int subscriber_socket = keyvalstore[i].subscribers[j];

                // Prepare the message to send to the subscriber
                char response[BUF_SIZE];
                snprintf(response, BUF_SIZE, "PUT:%s:%s\n", key, value);

                // Send the message to the subscriber
                send(subscriber_socket, response, strlen(response), 0);
            }
            return;
        }
    }
}



int get(char* key, char* res, int client_socket, char* client_name) {

    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            strcpy(res, keyvalstore[i].value);

            //Initialize the function
            get_successful(key, res);

            printf("Client %s procured %s\n", client_name, key);

            return 0;
        }
    }

    return -1;
}

void get_successful(char* key, char* value) {
    // Iterate over the keys
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            // Iterate over the subscribers of the key
            for (int j = 0; j < keyvalstore[i].num_subscribers; j++) {
                int subscriber_socket = keyvalstore[i].subscribers[j];

                // Prepare the message to send to the subscriber
                char response[BUF_SIZE];
                snprintf(response, BUF_SIZE, "GET:%s:%s\n", key, value);

                // Send the message to the subscriber
                send(subscriber_socket, response, strlen(response), 0);
            }
            return;
        }
    }
}



int del(char* key, char* value, int client_socket, char* client_name) {
    // Iterate over the keys
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            // Notify the subscribers of the key
            del_successful(key, value);

            // Delete the key-value pair
            keyvalstore[i].used = 0;
            strcpy(value, keyvalstore[i].value);

            // Print the delete operation details
            printf("Client %s deleted %s\n", client_name, key);
            return 0;
        }
    }

    // Key not found
    return -1;
}


void del_successful(char* key, char* value) {
    // Iterate over the keys
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            // Iterate over the subscribers of the key
            for (int j = 0; j < keyvalstore[i].num_subscribers; j++) {
                int subscriber_socket = keyvalstore[i].subscribers[j];

                // Prepare the message to send to the subscriber
                char response[BUF_SIZE];
                snprintf(response, BUF_SIZE, "DEL:%s:%s\n", key, value);

                // Send the message to the subscriber
                send(subscriber_socket, response, strlen(response), 0);
            }
            return;
        }
    }
}



void subscribe(int client_socket, char* key, char* client_name) {
    // Find the key in the key-value store
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            // Add the subscriber's socket to the list of subscribers
            int num_subscribers = keyvalstore[i].num_subscribers;
            if (num_subscribers < MAX_SUBSCRIBERS) {
                keyvalstore[i].subscribers[num_subscribers] = client_socket;
                keyvalstore[i].num_subscribers++;
                printf("Client %s subscribed to %s\n", client_name, key);
            } else {
                printf("Max subscribers reached for key %s\n", key);
            }
            return;
        }
    }

    // Key not found
    printf("Key %s not found\n", key);
}



