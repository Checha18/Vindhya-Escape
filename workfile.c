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

        // Check all four directions (up, right, down, left)
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
    
    // Start at (1,1) to ensure border walls
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
    maze[MAZE_ROWS-2][MAZE_COLS-2] = 0;  // exit
    }

    // Function to check visibility
    int isVisible(int x, int y, int playerX, int playerY) {
        int dx = x - (playerX + PIXEL_SIZE/2);
        int dy = y - (playerY + PIXEL_SIZE/2);
        return (dx * dx + dy * dy) <= (VISIBILITY_RADIUS * VISIBILITY_RADIUS);
    }

     SDL_Texture* loadTexture(const char* filePath, SDL_Renderer* renderer) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, filePath);
        if (!texture) {
            SDL_Log("Failed to load texture: %s", IMG_GetError());
        }
        return texture;
    }



    void startMazeGame() {
	if(init_client("192.168.27.183")<0){printf("Failed to connect to a server.");
	return;}
	int seed = giveseed();
            srand(seed);  // Initialize random seed
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

    // Load wall texture
    SDL_Texture *wallTexture = IMG_LoadTexture(renderer, "texture1.jpg");
    if (wallTexture == NULL) {
        printf("Failed to load texture! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    // Generate the maze
    init_maze();
    generate_maze();

    // Initialize player position to the entrance
    int pixelX = CELL_SIZE_X;
    int pixelY = CELL_SIZE_Y;
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
                    case SDLK_r:  // Add reset functionality
                        init_maze();
                        generate_maze();
                        pixelX = CELL_SIZE_X;
                        pixelY = CELL_SIZE_Y;
                        continue;
                }
            }
        }

        int flag = 1;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the maze walls that are within visibility circle
        for (int row = 0; row < MAZE_ROWS; row++) {
            for (int col = 0; col < MAZE_COLS; col++) {
                if (maze[row][col] == 1) {
                    SDL_Rect wallRect = {
                        col * CELL_SIZE_X,
                        row * CELL_SIZE_Y,
                        CELL_SIZE_X,
                        CELL_SIZE_Y
                    };

                    int isWallVisible = 
                        isVisible(col * CELL_SIZE_X, row * CELL_SIZE_Y, pixelX, pixelY) ||
                        isVisible((col + 1) * CELL_SIZE_X, row * CELL_SIZE_Y, pixelX, pixelY) ||
                        isVisible(col * CELL_SIZE_X, (row + 1) * CELL_SIZE_Y, pixelX, pixelY) ||
                        isVisible((col + 1) * CELL_SIZE_X, (row + 1) * CELL_SIZE_Y, pixelX, pixelY);

                    if (isWallVisible) {
                        // Render the wall texture
                        SDL_RenderCopy(renderer, wallTexture, NULL, &wallRect);
                    }

                    SDL_Rect newPixelRect = { newPixelX, newPixelY, PIXEL_SIZE, PIXEL_SIZE };
                    if (SDL_HasIntersection(&newPixelRect, &wallRect) && flag == 1) {
                        flag = 0;
                    }
                }
            }
        }

        // Create darkness overlay except for visibility circle
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                if (!isVisible(x, y, pixelX, pixelY)) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }

        if(flag) {
            pixelX = newPixelX;
            pixelY = newPixelY;
        }
	send_position(pixelX, pixelY);
	receive_game_state();
        // Draw the player
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pixel);

	
	SDL_Rect other_pixel = {other_player_pos.x, other_player_pos.y, PIXEL_SIZE, PIXEL_SIZE};
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(renderer, &other_pixel);
	SDL_RenderPresent(renderer);
    }

    // Clean up
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    }

    int main(int argc, char* argv[]) {
/*        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();

        SDL_Window* window = SDL_CreateWindow("Game Opening Screen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        TTF_Font* font = TTF_OpenFont("opensans.ttf", 24); // Change to your font file
        if (!font) {
            printf("Failed to load font: %s\n", TTF_GetError());
            return 1;
        }

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color buttonColor = {50, 150, 250, 255};

        // Title setup
        SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "VINDHYA ESCAPE", white);
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        SDL_FreeSurface(titleSurface);
        SDL_Rect titleRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4 - 25, 200, 50};

        SDL_Texture* cobwebTexture = loadTexture("bg.png", renderer);
        if (!cobwebTexture) {
            TTF_CloseFont(font);
            TTF_Quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return 1;
        }

        //Get the dimensions of the cobweb texture
        int cobwebWidth, cobwebHeight;
        SDL_QueryTexture(cobwebTexture, NULL, NULL, &cobwebWidth, &cobwebHeight);
        SDL_Rect cobwebRect = {0, 0, cobwebWidth, cobwebHeight}; // Position at (0, 0)
        SDL_RenderCopy(renderer, cobwebTexture, NULL, &cobwebRect);
        // Start Button setup
        SDL_Rect startButtonRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 200, 50};
        SDL_Surface* buttonSurface = TTF_RenderText_Solid(font, "Start Game", white);
        SDL_Texture* buttonTexture = SDL_CreateTextureFromSurface(renderer, buttonSurface);
        SDL_FreeSurface(buttonSurface);

        // Main loop for the opening screen
        int running = 1;
        SDL_Event event; 

        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
        Mix_Music *backgroundMusic = Mix_LoadMUS("bgmusic.mp3");
        Mix_PlayMusic(backgroundMusic, -1);

        while (running) {
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    running = 0;
                }

                // Check if the mouse is clicked
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                        mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h) {
                        startMazeGame(); // Start the maze game
                    }
                }
            }

            // Draw the opening screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
            SDL_RenderClear(renderer);

            // Draw title
            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

            // Draw button
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &startButtonRect);
            SDL_RenderCopy(renderer, buttonTexture, NULL, &startButtonRect);

            SDL_RenderPresent(renderer);
        }

        // Cleanup
        SDL_DestroyTexture(titleTexture);
        SDL_DestroyTexture(buttonTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();*/
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
        Mix_Music *backgroundMusic = Mix_LoadMUS("bgmusic.mp3");
        Mix_PlayMusic(backgroundMusic, -1);
	startMazeGame();
        return 0;
    }
