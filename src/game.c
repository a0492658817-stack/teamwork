#include "game.h"

#include <stdlib.h>
#include <string.h>

static const char *const kPieces[32] = {
    "redmaster.bmp",
    "redknight.bmp", "redknight.bmp",
    "redelephant.bmp", "redelephant.bmp",
    "redcar.bmp", "redcar.bmp",
    "redhorse.bmp", "redhorse.bmp",
    "redcannon.bmp", "redcannon.bmp",
    "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp",
    "blackmaster.bmp",
    "blackknight.bmp", "blackknight.bmp",
    "blackelephant.bmp", "blackelephant.bmp",
    "blackcar.bmp", "blackcar.bmp",
    "blackhorse.bmp", "blackhorse.bmp",
    "blackcannon.bmp", "blackcannon.bmp",
    "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp"
};

const char *image_path_root(void) {
    return "data/images";
}

void compose_image_path(const char *filename, char *full_path, size_t full_path_size) {
    if (full_path_size == 0) {
        return;
    }

    strncpy(full_path, image_path_root(), full_path_size - 1);
    full_path[full_path_size - 1] = '\0';
    strncat(full_path, "/", full_path_size - strlen(full_path) - 1);
    strncat(full_path, filename, full_path_size - strlen(full_path) - 1);
}

void click_to_cell(int mouse_x, int mouse_y, int *row, int *col) {
    if (mouse_x < TEAMWORK_START_X || mouse_y < TEAMWORK_START_Y) {
        *row = -1;
        *col = -1;
        return;
    }

    *col = (mouse_x - TEAMWORK_START_X) / TEAMWORK_CELL;
    *row = (mouse_y - TEAMWORK_START_Y) / TEAMWORK_CELL;
}

bool in_board(int row, int col) {
    return row >= 0 && row < TEAMWORK_ROWS && col >= 0 && col < TEAMWORK_COLS;
}

bool all_revealed(const GameState *state) {
    int row;
    int col;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) {
                return false;
            }
        }
    }
    return true;
}

bool player_flip_from_click(GameState *state, int mouse_x, int mouse_y) {
    int row;
    int col;

    click_to_cell(mouse_x, mouse_y, &row, &col);
    if (!in_board(row, col) || state->revealed[row][col]) {
        return false;
    }

    state->revealed[row][col] = true;
    return true;
}

void init_board(GameState *state) {
    int indices[32];
    int index;
    int row;
    int col;
    int piece_index;

    for (index = 0; index < 32; ++index) {
        indices[index] = index;
    }

    for (index = 0; index < 32; ++index) {
        int swap_index = rand() % 32;
        int temporary = indices[index];
        indices[index] = indices[swap_index];
        indices[swap_index] = temporary;
    }

    piece_index = 0;
    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            strcpy(state->board[row][col], kPieces[indices[piece_index++]]);
            state->revealed[row][col] = false;
        }
    }
}

bool computer_flip(GameState *state) {
    int hidden_rows[TEAMWORK_ROWS * TEAMWORK_COLS];
    int hidden_cols[TEAMWORK_ROWS * TEAMWORK_COLS];
    int hidden_count;
    int row;
    int col;

    hidden_count = 0;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) {
                hidden_rows[hidden_count] = row;
                hidden_cols[hidden_count] = col;
                ++hidden_count;
            }
        }
    }

    if (hidden_count == 0) {
        return false;
    }

    {
        int pick = rand() % hidden_count;
        state->revealed[hidden_rows[pick]][hidden_cols[pick]] = true;
    }
    return true;
}