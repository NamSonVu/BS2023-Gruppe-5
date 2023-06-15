#ifndef KEY_VAL_STORE_H
#define KEY_VAL_STORE_H

#include "main.h"

#define MAX_KEY_LEN  256
#define MAX_VALUE_LEN  1024
#define MAX_KEYS  100
#define MAX_SUBSCRIPTIONS 100

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int used;
} KeyValue;

void initKeyValStore();
int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

typedef struct {
    int used;
    int sockfd;
    char key[BUF_SIZE];
} Subscription;

int subscribe(int sockfd, char* key);
int unsubscribe(int sockfd, char* key);
void publish(char* key, char* value);
#endif
