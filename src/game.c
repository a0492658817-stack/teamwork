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
    /* 第六步：翻棋成功，步數加一 */
    state->move_count++;
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
    state->selected_row = -1;
    state->selected_col = -1;
    /* 第六步：初始化總步數為 0 */
    state->move_count = 0;
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
        /* 第六步：電腦翻棋成功，步數加一 */
        state->move_count++;
    }
    return true;
}

bool can_move(const GameState *state,
              int from_row, int from_col,
              int to_row,   int to_col) {
    int dr;
    int dc;

    if (!in_board(from_row, from_col) || !in_board(to_row, to_col)) {
        return false;
    }
    if (!state->revealed[from_row][from_col]) {
        return false;
    }
    if (state->board[from_row][from_col][0] == '\0') {
        return false;
    }

    dr = to_row - from_row;
    dc = to_col - from_col;

    if (!((dr == 1 && dc == 0) || (dr == -1 && dc == 0) ||
          (dr == 0 && dc == 1) || (dr == 0 && dc == -1))) {
        return false;
    }

    if (!state->revealed[to_row][to_col]) {
        return false;
    }
    if (state->board[to_row][to_col][0] != '\0') {
        return false;
    }

    return true;
}

bool move_piece(GameState *state,
                int from_row, int from_col,
                int to_row,   int to_col) {
    if (!can_move(state, from_row, from_col, to_row, to_col)) {
        return false;
    }

    strcpy(state->board[to_row][to_col], state->board[from_row][from_col]);
    state->board[from_row][from_col][0] = '\0';
    state->revealed[to_row][to_col]   = true;
    state->revealed[from_row][from_col] = true;
    /* 第六步：移動成功，步數加一 */
    state->move_count++;
    return true;
}

bool player_select_or_move(GameState *state, int mouse_x, int mouse_y) {
    int row;
    int col;

    click_to_cell(mouse_x, mouse_y, &row, &col);
    if (!in_board(row, col)) {
        return false;
    }

    if (state->selected_row == -1) {
        if (!state->revealed[row][col]) {
            state->revealed[row][col] = true;
            /* 第六步：玩家直接翻棋，步數加一 */
            state->move_count++;
            return true;
        }
        if (state->board[row][col][0] != '\0') {
            state->selected_row = row;
            state->selected_col = col;
            return false;
        }
        return false;
    }

    if (row == state->selected_row && col == state->selected_col) {
        state->selected_row = -1;
        state->selected_col = -1;
        return false;
    }

    if (move_piece(state, state->selected_row, state->selected_col, row, col)) {
        state->selected_row = -1;
        state->selected_col = -1;
        /* 注意：move_count 已經在 move_piece 函式內增加，這裡不需要重複加 */
        return true;
    }

    if (state->revealed[row][col] && state->board[row][col][0] != '\0') {
        state->selected_row = row;
        state->selected_col = col;
    } else {
        state->selected_row = -1;
        state->selected_col = -1;
    }
    return false;
}

bool computer_move(GameState *state) {
    int row;
    int col;
    int dr;
    int dc;
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int movable_from_rows[TEAMWORK_ROWS * TEAMWORK_COLS];
    int movable_from_cols[TEAMWORK_ROWS * TEAMWORK_COLS];
    int movable_count;
    int pick;
    int d;

    movable_count = 0;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) continue;
            if (state->board[row][col][0] == '\0') continue;

            for (d = 0; d < 4; ++d) {
                int nr = row + dirs[d][0];
                int nc = col + dirs[d][1];
                if (can_move(state, row, col, nr, nc)) {
                    movable_from_rows[movable_count] = row;
                    movable_from_cols[movable_count] = col;
                    ++movable_count;
                    break;
                }
            }
        }
    }

    if (movable_count == 0) {
        return computer_flip(state);
    }

    pick = rand() % movable_count;
    row = movable_from_rows[pick];
    col = movable_from_cols[pick];

    d = rand() % 4;
    {
        int i;
        for (i = 0; i < 4; ++i) {
            int di = (d + i) % 4;
            int nr = row + dirs[di][0];
            int nc = col + dirs[di][1];
            if (move_piece(state, row, col, nr, nc)) {
                return true;
            }
        }
    }

    return computer_flip(state);
}