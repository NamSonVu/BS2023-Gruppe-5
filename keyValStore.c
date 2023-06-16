#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include "keyValStore.h"

pthread_mutex_t subscriptions_mutex = PTHREAD_MUTEX_INITIALIZER;

static KeyValue keyvalstore[MAX_KEYS];
static Subscription subscriptions[MAX_KEYS];
static int num_subscriptions = 0;

void initKeyValStore() {
    memset(keyvalstore, 0, sizeof(KeyValue) * MAX_KEYS);
    memset(subscriptions, 0, sizeof(Subscription) * MAX_KEYS);
    num_subscriptions = 0;
}

void notifySubscribers(const char* key, const char* value, const char* action) {
    char notification[BUF_SIZE];
    snprintf(notification, BUF_SIZE, "%s:%s:%s\n", action, key, value);

    for (int j = 0; j < num_subscriptions; j++) {
        if (strcmp(subscriptions[j].key, key) == 0) {
            send(subscriptions[j].sockfd, notification, strlen(notification), 0);
        }
    }
}

int put(char* key, char* value) {
    int modified = 0;

    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            strcpy(keyvalstore[i].value, value);
            modified = 1;
            break;
        }
    }

    if (modified) {
        notifySubscribers(key, value, "KEY_MODIFIED");
        return 0;
    }

    for (int i = 0; i < MAX_KEYS; i++) {
        if (!keyvalstore[i].used) {
            strcpy(keyvalstore[i].key, key);
            strcpy(keyvalstore[i].value, value);
            keyvalstore[i].used = 1;
            notifySubscribers(key, value, "KEY_ADDED");
            return 0;
        }
    }

    return -1;
}

int get(char* key, char* res) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            strcpy(res, keyvalstore[i].value);
            return 0;
        }
    }

    return -1;
}

int del(char* key) {
    int deleted = 0;

    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            keyvalstore[i].used = 0;
            deleted = 1;
            break;
        }
    }

    if (deleted) {
        notifySubscribers(key, "", "KEY_DELETED");
        return 0;
    }

    return -1;
}

int subscribe(char* key, int sockfd) {
    pthread_mutex_lock(&subscriptions_mutex);  // Lock the mutex to ensure exclusive access to the subscriptions array

    if (num_subscriptions >= MAX_KEYS) {
        pthread_mutex_unlock(&subscriptions_mutex);  // Unlock the mutex before returning
        return -1;  // Maximum number of subscriptions reached
    }

    strcpy(subscriptions[num_subscriptions].key, key);
    subscriptions[num_subscriptions].sockfd = sockfd;
    num_subscriptions++;

    pthread_mutex_unlock(&subscriptions_mutex);  // Unlock the mutex

    return 0;
}

