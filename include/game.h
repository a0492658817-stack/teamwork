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
    int  selected_row;
    int  selected_col;
} GameState;

void init_board(GameState *state);
void click_to_cell(int mouse_x, int mouse_y, int *row, int *col);
bool in_board(int row, int col);
bool all_revealed(const GameState *state);
bool player_flip_from_click(GameState *state, int mouse_x, int mouse_y);
bool computer_flip(GameState *state);
const char *image_path_root(void);
bool can_move(const GameState *state, int from_row, int from_col, int to_row,   int to_col);
bool move_piece(GameState *state, int from_row, int from_col, int to_row, int to_col);
bool player_select_or_move(GameState *state, int mouse_x, int mouse_y);
bool computer_move(GameState *state);
void compose_image_path(const char *filename, char *full_path, size_t full_path_size);

#endif