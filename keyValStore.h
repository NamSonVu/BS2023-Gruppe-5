#ifndef KEYVALUE_H
#define KEYVALUE_H



// The struct that represents a key-value pair
struct KeyValuePair {
    char* key;
    char* value;
};

// The function for adding or updating a key-value pair
int put(char* key, char* value);

// The function for retrieving a value associated with a given key
int get(char* key, char** value);

// The function for deleting a key-value pair
int del(char* key);

#endif // KEYVALUE_H
