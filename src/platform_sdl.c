#include "app.h"

#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL2/SDL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char filename[TEAMWORK_PIECE_NAME_LENGTH];
    SDL_Texture *texture;
} TextureCacheEntry;

typedef struct {
    SDL_Renderer *renderer;
    TextureCacheEntry *cache;
    int *cache_count;
} SDLUiContext;

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

static TeamworkUiEvent poll_sdl_event(void *context, int *mouse_x, int *mouse_y) {
    (void)context;

    while (true) {
        SDL_Event event;

        if (!SDL_PollEvent(&event)) {
            return TEAMWORK_UI_EVENT_NONE;
        }

        if (event.type == SDL_QUIT) {
            return TEAMWORK_UI_EVENT_QUIT;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            *mouse_x = event.button.x;
            *mouse_y = event.button.y;
            return TEAMWORK_UI_EVENT_CLICK;
        }
    }
}

static void delay_sdl(void *context, unsigned int milliseconds) {
    (void)context;

    SDL_Delay(milliseconds);
}

static void draw_board(void *context, const GameState *state) {
    SDLUiContext *ui = (SDLUiContext *)context;
    int row;
    int col;

    SDL_SetRenderDrawColor(ui->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ui->renderer);

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            draw_one_cell(ui->renderer, state, ui->cache, ui->cache_count, row, col);
        }
    }

    if (all_revealed(state)) {
        SDL_Rect banner = {TEAMWORK_START_X, TEAMWORK_START_Y + TEAMWORK_ROWS * TEAMWORK_CELL + 10, 280, 28};
        SDL_SetRenderDrawColor(ui->renderer, 255, 215, 0, 255);
        SDL_RenderDrawRect(ui->renderer, &banner);
    }

    SDL_RenderPresent(ui->renderer);
}

int teamwork_run_platform_app(void) {
    GameState state;
    TextureCacheEntry texture_cache[TEAMWORK_ROWS * TEAMWORK_COLS + 1];
    int texture_cache_count;
    SDLUiContext ui_context;
    TeamworkUi ui;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int player_first;
    int index;

    player_first = teamwork_prompt_turn_order();

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
    ui_context.renderer = renderer;
    ui_context.cache = texture_cache;
    ui_context.cache_count = &texture_cache_count;

    ui.context = &ui_context;
    ui.poll_event = poll_sdl_event;
    ui.draw_board = draw_board;
    ui.delay_ms = delay_sdl;

    teamwork_run_game_session(&state, &ui, player_first);

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