#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>
#include "client.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PIXEL_SIZE 15
#define MAZE_ROWS 40 // Number of rows in the maze array
#define MAZE_COLS 60 // Number of columns in the maze array
#define CELL_SIZE_X (SCREEN_WIDTH / MAZE_COLS) // Dynamic cell width
#define CELL_SIZE_Y (SCREEN_HEIGHT / MAZE_ROWS) // Dynamic cell height
#define VISIBILITY_RADIUS 150 // Increased for better visibility with scaled map

typedef struct {
    int x, y;
} Cell;

int maze[MAZE_ROWS][MAZE_COLS];

// Initialize maze with walls
void init_maze() {
    for (int row = 0; row < MAZE_ROWS; row++) {
        for (int col = 0; col < MAZE_COLS; col++) {
            maze[row][col] = 1; // All cells start as walls
        }
    }
}

// Get random unvisited neighbor
Cell get_random_neighbor(Cell current) {
    Cell neighbors[4];
    int count = 0;

    if (current.y > 1 && maze[current.y - 2][current.x] == 1) {
        neighbors[count++] = (Cell){current.x, current.y - 2};
    }
    if (current.x < MAZE_COLS - 2 && maze[current.y][current.x + 2] == 1) {
        neighbors[count++] = (Cell){current.x + 2, current.y};
    }
    if (current.y < MAZE_ROWS - 2 && maze[current.y + 2][current.x] == 1) {
        neighbors[count++] = (Cell){current.x, current.y + 2};
    }
    if (current.x > 1 && maze[current.y][current.x - 2] == 1) {
        neighbors[count++] = (Cell){current.x - 2, current.y};
    }

    if (count > 0) {
        return neighbors[rand() % count];
    }
    return (Cell){-1, -1}; // No unvisited neighbors
}

// Carve path between two cells
void carve_path(Cell from, Cell to) {
    maze[to.y][to.x] = 0;
    maze[(from.y + to.y) / 2][(from.x + to.x) / 2] = 0;
}

// Generate maze using recursive backtracking
void generate_maze() {
    Cell stack[MAZE_ROWS * MAZE_COLS];
    int stack_size = 0;

    Cell start = {1, 1};
    maze[start.y][start.x] = 0;
    stack[stack_size++] = start;

    while (stack_size > 0) {
        Cell current = stack[stack_size - 1];
        Cell neighbor = get_random_neighbor(current);

        if (neighbor.x != -1) {
            carve_path(current, neighbor);
            stack[stack_size++] = neighbor;
        } else {
            stack_size--;
        }
    }
    maze[1][1] = 0;  // entrance
    maze[MAZE_ROWS - 2][MAZE_COLS - 2] = 0;  // exit
}

// Function to check visibility
int isVisible(int x, int y, int playerX, int playerY) {
    int dx = x - (playerX + PIXEL_SIZE / 2);
    int dy = y - (playerY + PIXEL_SIZE / 2);
    return (dx * dx + dy * dy) <= (VISIBILITY_RADIUS * VISIBILITY_RADIUS);
}

// Load texture
SDL_Texture* loadTexture(const char* filePath, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, filePath);
    if (!texture) {
        SDL_Log("Failed to load texture: %s", IMG_GetError());
    }
    return texture;
}

// Function to find an open cell near the right side of the maze
Cell findOpenCellOnRight() {
    for (int row = MAZE_ROWS / 2; row < MAZE_ROWS; row++) {
        for (int col = MAZE_COLS - 2; col >= MAZE_COLS / 2; col--) {
            if (maze[row][col] == 0) { // Check if cell is open
                return (Cell){col * CELL_SIZE_X, row * CELL_SIZE_Y};
            }
        }
    }
    return (Cell){CELL_SIZE_X, CELL_SIZE_Y};
}

// Start the maze game
void startMazeGame() {
    if(init_client("192.168.27.183") < 0) {
        printf("Failed to connect to a server.");
        return;
    }
    int seed = giveseed();
    srand(seed);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Vindhya Escape",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Texture *wallTexture = IMG_LoadTexture(renderer, "texture1.jpg");
    if (wallTexture == NULL) {
        printf("Failed to load texture! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    init_maze();
    generate_maze();

    Cell startCell = findOpenCellOnRight();
    int pixelX = startCell.x;
    int pixelY = startCell.y;
    int playerSpeed = CELL_SIZE_X / 6;
    int running = 1;
    SDL_Event event;

    while (running) {
        SDL_Rect pixel = { pixelX, pixelY, PIXEL_SIZE, PIXEL_SIZE };
        int newPixelX = pixelX;
        int newPixelY = pixelY;

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        newPixelX -= playerSpeed;
                        break;
                    case SDLK_d:
                        newPixelX += playerSpeed;
                        break;
                    case SDLK_w:
                        newPixelY -= playerSpeed;
                        break;
                    case SDLK_s:
                        newPixelY += playerSpeed;
                        break;
                    case SDLK_r:
                        init_maze();
                        generate_maze();
                        startCell = findOpenCellOnRight();
                        pixelX = startCell.x;
                        pixelY = startCell.y;
                        continue;
                }
            }
        }

        int flag = 1;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int row = 0; row < MAZE_ROWS; row++) {
            for (int col = 0; col < MAZE_COLS; col++) {
                if (maze[row][col] == 1) {
                    SDL_Rect wallRect = {
                        col * CELL_SIZE_X,
                        row * CELL_SIZE_Y,
                        CELL_SIZE_X,
                        CELL_SIZE_Y
                    };

                    SDL_RenderCopy(renderer, wallTexture, NULL, &wallRect);

                    SDL_Rect newPixelRect = { newPixelX, newPixelY, PIXEL_SIZE, PIXEL_SIZE };
                    if (SDL_HasIntersection(&newPixelRect, &wallRect) && flag == 1) {
                        flag = 0;
                    }
                }
            }
        }

        if(flag) {
            pixelX = newPixelX;
            pixelY = newPixelY;
        }

        send_position(pixelX, pixelY);
        receive_game_state();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pixel);

        SDL_Rect other_pixel = {other_player_pos.x, other_player_pos.y, PIXEL_SIZE, PIXEL_SIZE};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &other_pixel);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(wallTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music *backgroundMusic = Mix_LoadMUS("bgmusic.mp3");
    Mix_PlayMusic(backgroundMusic, -1);
    startMazeGame();
    return 0;
}

