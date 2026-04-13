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

#if defined(TEAMWORK_USE_SDL2_TTF)
#if defined(__has_include)
#if __has_include(<SDL2/SDL_ttf.h>)
#include <SDL2/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif
#else
#include <SDL2/SDL_ttf.h>
#endif
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

#if defined(TEAMWORK_USE_SDL2_TTF)
static TTF_Font *load_turn_order_font(void) {
    static const char *font_paths[] = {
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STHeiti Light.ttc",
        "/Library/Fonts/Arial Unicode.ttf"
    };
    int i;

    for (i = 0; i < (int)(sizeof(font_paths) / sizeof(font_paths[0])); ++i) {
        TTF_Font *font = TTF_OpenFont(font_paths[i], 28);
        if (font != NULL) {
            return font;
        }
    }

    return NULL;
}

static void draw_text_center(SDL_Renderer *renderer,
                             TTF_Font *font,
                             const char *text,
                             SDL_Rect rect,
                             SDL_Color color) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dst;

    if (font == NULL || text == NULL || text[0] == '\0') {
        return;
    }

    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == NULL) {
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_FreeSurface(surface);
        return;
    }

    dst.w = surface->w;
    dst.h = surface->h;
    dst.x = rect.x + (rect.w - dst.w) / 2;
    dst.y = rect.y + (rect.h - dst.h) / 2;

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}
#endif

static int prompt_turn_order_click_sdl(SDL_Renderer *renderer) {
    SDL_Rect player_btn = {140, 180, 220, 90};
    SDL_Rect computer_btn = {440, 180, 220, 90};
#if defined(TEAMWORK_USE_SDL2_TTF)
    TTF_Font *font = NULL;
#endif

    printf("[LOG] Click left box for Player First, right box for Computer First.\n");

#if defined(TEAMWORK_USE_SDL2_TTF)
    if (TTF_Init() == 0) {
        font = load_turn_order_font();
    }
#endif

    while (true) {
        SDL_Event event;

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 160, 80, 255);
        SDL_RenderFillRect(renderer, &player_btn);
        SDL_SetRenderDrawColor(renderer, 160, 80, 0, 255);
        SDL_RenderFillRect(renderer, &computer_btn);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &player_btn);
        SDL_RenderDrawRect(renderer, &computer_btn);

    #if defined(TEAMWORK_USE_SDL2_TTF)
        {
            SDL_Color text_color = {255, 255, 255, 255};
            draw_text_center(renderer, font, "玩家先手", player_btn, text_color);
            draw_text_center(renderer, font, "電腦先手", computer_btn, text_color);
        }
    #endif

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
#if defined(TEAMWORK_USE_SDL2_TTF)
                if (font != NULL) {
                    TTF_CloseFont(font);
                }
                if (TTF_WasInit()) {
                    TTF_Quit();
                }
#endif
                return 1;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int x = event.button.x;
                int y = event.button.y;

                if (x >= player_btn.x && x < player_btn.x + player_btn.w &&
                    y >= player_btn.y && y < player_btn.y + player_btn.h) {
                    printf("[LOG] Turn order selected: Player first.\n");
#if defined(TEAMWORK_USE_SDL2_TTF)
                    if (font != NULL) {
                        TTF_CloseFont(font);
                    }
                    if (TTF_WasInit()) {
                        TTF_Quit();
                    }
#endif
                    return 1;
                }

                if (x >= computer_btn.x && x < computer_btn.x + computer_btn.w &&
                    y >= computer_btn.y && y < computer_btn.y + computer_btn.h) {
                    printf("[LOG] Turn order selected: Computer first.\n");
#if defined(TEAMWORK_USE_SDL2_TTF)
                    if (font != NULL) {
                        TTF_CloseFont(font);
                    }
                    if (TTF_WasInit()) {
                        TTF_Quit();
                    }
#endif
                    return 2;
                }
            }
        }

        SDL_Delay(16);
    }
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

    player_first = prompt_turn_order_click_sdl(renderer);

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