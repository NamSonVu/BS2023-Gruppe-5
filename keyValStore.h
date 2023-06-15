#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#define MAX_KEYS 100
#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 256
#define MAX_SUBSCRIBERS 10
#define BUF_SIZE 1024

typedef struct {
    int used;
    int client_socket;
    char client_name[BUF_SIZE];
    char key[MAX_KEY_LEN];
} Subscription;

int put(char* key, char* value, int client_socket, char* client_name);
int get(char* key, char* res, int client_socket, char* client_name);
int del(char* key, char* value, int client_socket, char* client_name);
void subscribe(int client_socket, char* key, char* client_name);
void put_successful(char* key, char* value);
void get_successful(char* key, char* value);
void del_successful(char* key, char* value);
void initKeyValStore();

void generate_client_name(char* client_name);



#endif

