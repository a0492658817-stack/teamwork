#include "../include/game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <graphics.h>

static void draw_one_cell(const GameState *state, int row, int col) {
    int x1 = TEAMWORK_START_X + col * TEAMWORK_CELL;
    int y1 = TEAMWORK_START_Y + row * TEAMWORK_CELL;
    int x2 = x1 + TEAMWORK_CELL;
    int y2 = y1 + TEAMWORK_CELL;

    setcolor(WHITE);
    rectangle(x1, y1, x2, y2);

    int imgLeft = x1 + 5;
    int imgTop = y1 + 5;
    int imgRight = x2 - 5;
    int imgBottom = y2 - 5;

    char imagePath[128];
    if (!state->revealed[row][col]) {
        compose_image_path("background.bmp", imagePath, sizeof(imagePath));
    } else {
        compose_image_path(state->board[row][col], imagePath, sizeof(imagePath));
    }

    readimagefile(imagePath, imgLeft, imgTop, imgRight, imgBottom);
}

static void draw_board(const GameState *state) {
    cleardevice();

    setcolor(WHITE);
    outtextxy(50, 20, "Dark Chess");
    outtextxy(170, 20, "Player click one covered piece");

    for (int row = 0; row < TEAMWORK_ROWS; ++row) {
        for (int col = 0; col < TEAMWORK_COLS; ++col) {
            draw_one_cell(state, row, col);
        }
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    int playerFirst;
    printf("Choose turn order:\n");
    printf("1. Player first\n");
    printf("2. Computer first\n");
    printf("Enter: ");
    scanf("%d", &playerFirst);

    initwindow(TEAMWORK_WIN_W, TEAMWORK_WIN_H, "Dark Chess");
    setbkcolor(BLACK);
    cleardevice();

    GameState state;
    init_board(&state);
    draw_board(&state);

    if (playerFirst == 2) {
        delay(500);
        computer_flip(&state);
        draw_board(&state);
    }

    while (true) {
        if (all_revealed(&state)) {
            setcolor(YELLOW);
            outtextxy(50, 400, "All pieces are revealed.");
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mouseX;
            int mouseY;
            getmouseclick(WM_LBUTTONDOWN, mouseX, mouseY);

            if (player_flip_from_click(&state, mouseX, mouseY)) {
                draw_board(&state);

                if (!all_revealed(&state)) {
                    delay(500);
                    computer_flip(&state);
                    draw_board(&state);
                }
            }
        }

        delay(30);
    }

    getch();
    closegraph();
    return 0;
}
#elif defined(TEAMWORK_USE_SDL2)
#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL2/SDL.h>
#endif

typedef struct {
    char filename[TEAMWORK_PIECE_NAME_LENGTH];
    SDL_Texture *texture;
} TextureCacheEntry;

static SDL_Texture *load_texture(SDL_Renderer *renderer, TextureCacheEntry *cache, int *cache_count, const char *filename) {
    int index;
    char imagePath[128];

    for (index = 0; index < *cache_count; ++index) {
        if (strcmp(cache[index].filename, filename) == 0) {
            return cache[index].texture;
        }
    }

    compose_image_path(filename, imagePath, sizeof(imagePath));

    {
        SDL_Surface *surface = SDL_LoadBMP(imagePath);
        SDL_Texture *texture;
        if (surface == NULL) {
            return NULL;
        }

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (texture == NULL) {
            return NULL;
        }

        strncpy(cache[*cache_count].filename, filename, TEAMWORK_PIECE_NAME_LENGTH - 1);
        cache[*cache_count].filename[TEAMWORK_PIECE_NAME_LENGTH - 1] = '\0';
        cache[*cache_count].texture = texture;
        ++(*cache_count);
        return texture;
    }
}

static void draw_one_cell(SDL_Renderer *renderer, const GameState *state, TextureCacheEntry *cache, int *cache_count, int row, int col) {
    int x1 = TEAMWORK_START_X + col * TEAMWORK_CELL;
    int y1 = TEAMWORK_START_Y + row * TEAMWORK_CELL;
    SDL_Rect border = {x1, y1, TEAMWORK_CELL, TEAMWORK_CELL};
    SDL_Rect dst = {x1 + 5, y1 + 5, TEAMWORK_CELL - 10, TEAMWORK_CELL - 10};
    SDL_Texture *texture;
    const char *filename;

    filename = state->revealed[row][col] ? state->board[row][col] : "background.bmp";
    texture = load_texture(renderer, cache, cache_count, filename);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &border);

    if (texture != NULL) {
        SDL_RenderCopy(renderer, texture, NULL, &dst);
    } else {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderFillRect(renderer, &dst);
    }
}

static void draw_board(SDL_Renderer *renderer, const GameState *state, TextureCacheEntry *cache, int *cache_count) {
    int row;
    int col;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            draw_one_cell(renderer, state, cache, cache_count, row, col);
        }
    }

    if (all_revealed(state)) {
        SDL_Rect banner = {TEAMWORK_START_X, TEAMWORK_START_Y + TEAMWORK_ROWS * TEAMWORK_CELL + 10, 280, 28};
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        SDL_RenderDrawRect(renderer, &banner);
    }

    SDL_RenderPresent(renderer);
}

int main(void) {
    GameState state;
    TextureCacheEntry texture_cache[TEAMWORK_ROWS * TEAMWORK_COLS + 1];
    int texture_cache_count;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int playerFirst;
    bool running;
    int index;

    srand((unsigned)time(NULL));

    printf("Choose turn order:\n");
    printf("1. Player first\n");
    printf("2. Computer first\n");
    printf("Enter: ");
    if (scanf("%d", &playerFirst) != 1) {
        playerFirst = 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "Dark Chess",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        TEAMWORK_WIN_W,
        TEAMWORK_WIN_H,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    init_board(&state);
    texture_cache_count = 0;
    draw_board(renderer, &state, texture_cache, &texture_cache_count);

    if (playerFirst == 2) {
        SDL_Delay(500);
        computer_flip(&state);
        draw_board(renderer, &state, texture_cache, &texture_cache_count);
    }

    running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (player_flip_from_click(&state, event.button.x, event.button.y)) {
                    draw_board(renderer, &state, texture_cache, &texture_cache_count);

                    if (!all_revealed(&state)) {
                        SDL_Delay(500);
                        computer_flip(&state);
                        draw_board(renderer, &state, texture_cache, &texture_cache_count);
                    }
                }
            }
        }

        SDL_Delay(16);
    }

    for (index = 0; index < texture_cache_count; ++index) {
        if (texture_cache[index].texture != NULL) {
            SDL_DestroyTexture(texture_cache[index].texture);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
#else
int main(void) {
    srand((unsigned)time(NULL));

    GameState state;
    init_board(&state);

    printf("Dark Chess core initialized on a non-Windows platform.\n");
    printf("Board size: %d x %d\n", TEAMWORK_ROWS, TEAMWORK_COLS);
    printf("Assets root: %s\n", image_path_root());
    printf("All pieces revealed: %s\n", all_revealed(&state) ? "yes" : "no");

    return 0;
}
#endif
