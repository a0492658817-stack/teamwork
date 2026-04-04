#ifndef TEAMWORK_GAME_H
#define TEAMWORK_GAME_H

#include <stdbool.h>
#include <stddef.h>

#define TEAMWORK_ROWS 4
#define TEAMWORK_COLS 8
#define TEAMWORK_CELL 80
#define TEAMWORK_START_X 50
#define TEAMWORK_START_Y 50
#define TEAMWORK_WIN_W 800
#define TEAMWORK_WIN_H 500
#define TEAMWORK_PIECE_NAME_LENGTH 40

typedef struct {
    char board[TEAMWORK_ROWS][TEAMWORK_COLS][TEAMWORK_PIECE_NAME_LENGTH];
    bool revealed[TEAMWORK_ROWS][TEAMWORK_COLS];
} GameState;

void init_board(GameState *state);
void click_to_cell(int mouse_x, int mouse_y, int *row, int *col);
bool in_board(int row, int col);
bool all_revealed(const GameState *state);
bool computer_flip(GameState *state);
const char *image_path_root(void);
void compose_image_path(const char *filename, char *full_path, size_t full_path_size);

#endif