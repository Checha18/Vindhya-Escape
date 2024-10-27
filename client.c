#include "client.h"

int client_socket;
int player_id;
int seed;
PlayerPosition other_player_pos = {0, 0};

int init_client(const char* server_ip) {
    struct sockaddr_in server_addr;
    
    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }
    
    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    // Receive player ID from server
    recv(client_socket, &player_id, sizeof(player_id), 0);
    recv(client_socket, &seed, sizeof(seed), 0);
    printf("Connected as Player %d\n", player_id);
    
    return 0;
}

void send_position(int x, int y) {
    PlayerPosition pos = {x, y};
    send(client_socket, &pos, sizeof(PlayerPosition), 0);
}

void receive_game_state(void) {
    PlayerPosition positions[2];
    int bytes_received = recv(client_socket, positions, sizeof(PlayerPosition) * 2, MSG_DONTWAIT);
    
    if (bytes_received == sizeof(PlayerPosition) * 2) {
        other_player_pos = positions[1 - player_id];
    }
}

void cleanup_client(void) {
    close(client_socket);
}

int giveseed(void){
    return seed;
}
