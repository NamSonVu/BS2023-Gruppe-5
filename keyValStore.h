// keyValStore.h

#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#define MAX_KEYS 100
#define BUF_SIZE 1024
#define MAX_SUBSCRIPTIONS 100

typedef struct {
    char key[BUF_SIZE];
    char value[BUF_SIZE];
    int used;
} KeyValue;

typedef struct {
    int sockfd;
    char key[BUF_SIZE];
} Subscription;

void initKeyValStore();
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);
int subscribe(char* key, int sockfd);

#endif
