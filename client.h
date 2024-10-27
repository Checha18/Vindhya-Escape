#ifndef CLIENT_H
#define CLIENT_H

#define PORT 8080
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    int x;
    int y;
} PlayerPosition;

extern int client_socket;
extern int player_id;
extern PlayerPosition other_player_pos;

int init_client(const char* server_ip);
void send_position(int x, int y);
void receive_game_state(void);
void cleanup_client(void);
int giveseed(void);

#endif
