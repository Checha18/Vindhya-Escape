#include <time.h>
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


static int server_socket;
static GameState game_state;

void init_server(void) {
    struct sockaddr_in server_addr;
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(1);
    }
    
    // Listen for connections
    if (listen(server_socket, 2) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    // Initialize game state
    game_state.connected_clients = 0;
    game_state.players[0] = (PlayerPosition){60, 60};
    game_state.players[1] = (PlayerPosition){740, 540};
    
    printf("Server initialized on port %d\n", PORT);
}

void handle_clients(void) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int seed = time(NULL);
    while (game_state.connected_clients < MAX_CLIENTS) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        game_state.client_sockets[game_state.connected_clients] = client_socket;
        
        // Send player ID to client
        int player_id = game_state.connected_clients;
        send(client_socket, &player_id, sizeof(player_id), 0);
	send(client_socket, &seed, sizeof(seed), 0);
        game_state.connected_clients++;
        printf("Player %d connected\n", player_id);
    }
    
    // Game loop
    while (1) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            PlayerPosition pos;
            int bytes_received = recv(game_state.client_sockets[i], &pos, sizeof(PlayerPosition), MSG_DONTWAIT);
            
            if (bytes_received == sizeof(PlayerPosition)) {
                game_state.players[i] = pos;
                if(game_state.players[0].x == game_state.players[1].x && game_state.players[0].y == game_state.players[1].y)
		{
			cleanup_server();
		}
                // Broadcast updated positions to all clients
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    send(game_state.client_sockets[j], &game_state.players, sizeof(PlayerPosition) * 2, 0);
                }
            }
        }
        usleep(16666); // ~60Hz update rate
    }
}

void cleanup_server(void) {
    for (int i = 0; i < game_state.connected_clients; i++) {
        close(game_state.client_sockets[i]);
    }
    close(server_socket);
}


int main()
{   
    
    init_server();
    handle_clients();
    cleanup_server();
    return 0;
}
