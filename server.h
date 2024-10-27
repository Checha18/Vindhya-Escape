/ server.h
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 2

typedef struct {
    int x;
    int y;
} PlayerPosition;

typedef struct {
    PlayerPosition players[2];
    int connected_clients;
    int client_sockets[2];
} GameState;

void init_server(void);
void handle_clients(void);
void cleanup_server(void);

#endif
