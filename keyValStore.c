#include "keyValStore.h"
#include <stdlib.h>
#include <string.h>
#define MAX_DATA_SIZE 1000

typedef struct {
    char* key;
    char* value;
} KeyValuePair;

KeyValuePair data[MAX_DATA_SIZE];
int dataCount = 0;


int put(char* key, char* value) {
    // Check if the key already exists
    for (int i = 0; i < dataCount; i++) {
        if (strcmp(data[i].key, key) == 0) {
            // Key already exists, overwrite value
            data[i].value = value;
            return 1;
        }
    }

    // Key doesn't exist, add new key-value pair
    if (dataCount < MAX_DATA_SIZE) {
        KeyValuePair kvp = {key, value};
        data[dataCount] = kvp;
        dataCount++;
        return 1;
    }

    // Data array is full
    return -1;
}


int get(char* key, char** res) {
    // Search for key in data array
    for (int i = 0; i < dataCount; i++) {
        if (strcmp(data[i].key, key) == 0) {
            // Key found, set result parameter and return 1
            *res = data[i].value;
            return 1;
        }
    }

    // Key not found
    return -1;
}


int del(char* key) {
    // Search for key in data array
    for (int i = 0; i < dataCount; i++) {
        if (strcmp(data[i].key, key) == 0) {
            // Key found, remove key-value pair from data array
            for (int j = i; j < dataCount - 1; j++) {
                data[j] = data[j+1];
            }
            dataCount--;
            return 1;
        }
    }

    // Key not found
    return -1;


}




