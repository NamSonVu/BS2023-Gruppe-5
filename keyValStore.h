#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#define MAX_KEYS 100
#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 256

int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);


void initKeyValStore();

#endif