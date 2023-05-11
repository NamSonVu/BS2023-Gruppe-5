#include <string.h>
#include "keyValStore.h"

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int used;
} KeyValue;

static KeyValue keyvalstore[MAX_KEYS];

void initKeyValStore() {
    memset(keyvalstore, 0, sizeof(KeyValue) * MAX_KEYS);
}

int put(char* key, char* value) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            strcpy(keyvalstore[i].value, value);
            return 0;
        }
    }

    for (int i = 0; i < MAX_KEYS; i++) {
        if (!keyvalstore[i].used) {
            strcpy(keyvalstore[i].key, key);
            strcpy(keyvalstore[i].value, value);
            keyvalstore[i].used = 1;
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
    for (int i = 0; i < MAX_KEYS; i++) {
        if (keyvalstore[i].used && strcmp(keyvalstore[i].key, key) == 0) {
            keyvalstore[i].used = 0;
            return 0;
        }
    }

    return -1;
}
