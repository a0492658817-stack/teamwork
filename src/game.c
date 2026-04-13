#include "game.h"
#include <stdlib.h>
#include <string.h>

static int piece_camp(const char *piece_name) {
    if (piece_name == NULL || piece_name[0] == '\0') {
        return 0;
    }
    if (strncmp(piece_name, "red", 3) == 0) {
        return 1;
    }
    if (strncmp(piece_name, "black", 5) == 0) {
        return 2;
    }
    return 0;
}

static int piece_rank(const char *piece_name) {
    if (piece_name == NULL || piece_name[0] == '\0') {
        return 0;
    }
    if (strstr(piece_name, "master") != NULL) return 7;
    if (strstr(piece_name, "knight") != NULL) return 6;
    if (strstr(piece_name, "elephant") != NULL) return 5;
    if (strstr(piece_name, "car") != NULL) return 4;
    if (strstr(piece_name, "horse") != NULL) return 3;
    if (strstr(piece_name, "cannon") != NULL) return 2;
    if (strstr(piece_name, "soldier") != NULL) return 1;
    return 0;
}

static bool is_computer_piece(const GameState *state, const char *piece_name) {
    return piece_camp(piece_name) == state->computer_camp;
}

static bool is_player_piece(const GameState *state, const char *piece_name) {
    return piece_camp(piece_name) == state->player_camp;
}

static void record_player_action(GameState *state) {
    state->player_moves++;
    state->move_count++;
}

static void record_computer_action(GameState *state) {
    state->computer_moves++;
    state->move_count++;
}

static bool is_adjacent_orthogonal(int from_row, int from_col, int to_row, int to_col) {
    int dr = to_row - from_row;
    int dc = to_col - from_col;

    return (dr == 1 && dc == 0) || (dr == -1 && dc == 0) ||
           (dr == 0 && dc == 1) || (dr == 0 && dc == -1);
}

static int count_intervening_pieces_on_line(const GameState *state,
                                            int from_row, int from_col,
                                            int to_row,   int to_col) {
    int screens;

    if (from_row == to_row) {
        int step = (to_col > from_col) ? 1 : -1;
        int c;

        screens = 0;
        for (c = from_col + step; c != to_col; c += step) {
            if (state->board[from_row][c][0] != '\0') {
                screens++;
            }
        }
        return screens;
    }

    if (from_col == to_col) {
        int step = (to_row > from_row) ? 1 : -1;
        int r;

        screens = 0;
        for (r = from_row + step; r != to_row; r += step) {
            if (state->board[r][from_col][0] != '\0') {
                screens++;
            }
        }
        return screens;
    }

    return -1;
}

static bool try_computer_capture_move(GameState *state) {
    int row;
    int col;
    int target_row;
    int target_col;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) {
                continue;
            }
            if (!is_computer_piece(state, state->board[row][col])) {
                continue;
            }

            for (target_row = 0; target_row < TEAMWORK_ROWS; ++target_row) {
                for (target_col = 0; target_col < TEAMWORK_COLS; ++target_col) {
                    if (!can_capture(state, row, col, target_row, target_col)) {
                        continue;
                    }
                    if (move_piece(state, row, col, target_row, target_col)) {
                        record_computer_action(state);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static bool is_threatened_at(const GameState *state, int row, int col) {
    const char *self_piece;
    int self_camp;
    int enemy_row;
    int enemy_col;

    if (!in_board(row, col) || !state->revealed[row][col]) {
        return false;
    }

    self_piece = state->board[row][col];
    if (self_piece[0] == '\0') {
        return false;
    }

    self_camp = piece_camp(self_piece);

    if (self_camp == 0) {
        return false;
    }

    for (enemy_row = 0; enemy_row < TEAMWORK_ROWS; ++enemy_row) {
        for (enemy_col = 0; enemy_col < TEAMWORK_COLS; ++enemy_col) {
            if (piece_camp(state->board[enemy_row][enemy_col]) == self_camp) {
                continue;
            }
            if (can_capture(state, enemy_row, enemy_col, row, col)) {
                return true;
            }
        }
    }

    return false;
}

static bool is_threatened_after_move(const GameState *state,
                                     int from_row, int from_col,
                                     int to_row,   int to_col) {
    GameState simulated;

    if (!in_board(to_row, to_col)) {
        return false;
    }
    if (!in_board(from_row, from_col)) {
        return false;
    }
    if (state->board[from_row][from_col][0] == '\0') {
        return false;
    }

    simulated = *state;
    strcpy(simulated.board[to_row][to_col], simulated.board[from_row][from_col]);
    simulated.board[from_row][from_col][0] = '\0';
    simulated.revealed[to_row][to_col] = true;
    simulated.revealed[from_row][from_col] = true;

    return is_threatened_at(&simulated, to_row, to_col);
}

static bool try_computer_evasion_move(GameState *state) {
    static const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int row;
    int col;
    int d;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) {
                continue;
            }
            if (!is_computer_piece(state, state->board[row][col])) {
                continue;
            }
            if (!is_threatened_at(state, row, col)) {
                continue;
            }

            for (d = 0; d < 4; ++d) {
                int nr = row + dirs[d][0];
                int nc = col + dirs[d][1];

                if (!can_move(state, row, col, nr, nc)) {
                    continue;
                }
                if (is_threatened_after_move(state, row, col, nr, nc)) {
                    continue;
                }
                if (move_piece(state, row, col, nr, nc)) {
                    record_computer_action(state);
                    return true;
                }
            }
        }
    }

    return false;
}

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
    record_player_action(state);
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
    state->move_count = 0;
    state->player_moves = 0;
    state->computer_moves = 0;
    state->player_camp = TEAMWORK_CAMP_RED;
    state->computer_camp = TEAMWORK_CAMP_BLACK;
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
        record_computer_action(state);
    }
    return true;
}

bool can_capture(const GameState *state,
                 int from_row, int from_col,
                 int to_row,   int to_col) {
    int from_camp;
    int to_camp;
    int from_rank;
    int to_rank;
    int screens;

    if (!in_board(from_row, from_col) || !in_board(to_row, to_col)) {
        return false;
    }
    if (!state->revealed[from_row][from_col] || !state->revealed[to_row][to_col]) {
        return false;
    }
    if (state->board[from_row][from_col][0] == '\0' || state->board[to_row][to_col][0] == '\0') {
        return false;
    }

    from_camp = piece_camp(state->board[from_row][from_col]);
    to_camp = piece_camp(state->board[to_row][to_col]);
    from_rank = piece_rank(state->board[from_row][from_col]);
    to_rank = piece_rank(state->board[to_row][to_col]);

    if (from_camp == 0 || to_camp == 0 || from_camp == to_camp) {
        return false;
    }

    if (from_rank == 2) {
        screens = count_intervening_pieces_on_line(state, from_row, from_col, to_row, to_col);
        return screens == 1;
    }

    if (!is_adjacent_orthogonal(from_row, from_col, to_row, to_col)) {
        return false;
    }

    if (from_rank == 1 && to_rank == 7) {
        return true;
    }

    if (from_rank == 7 && to_rank == 1) {
        return false;
    }

    return from_rank >= to_rank;
}

bool can_move(const GameState *state,
              int from_row, int from_col,
              int to_row,   int to_col) {
    if (!in_board(from_row, from_col) || !in_board(to_row, to_col)) {
        return false;
    }
    if (!state->revealed[from_row][from_col]) {
        return false;
    }
    if (state->board[from_row][from_col][0] == '\0') {
        return false;
    }

    if (!state->revealed[to_row][to_col]) {
        return false;
    }

    if (state->board[to_row][to_col][0] == '\0') {
        return is_adjacent_orthogonal(from_row, from_col, to_row, to_col);
    }

    return can_capture(state, from_row, from_col, to_row, to_col);
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
            record_player_action(state);
            return true;
        }
        if (state->board[row][col][0] != '\0' && is_player_piece(state, state->board[row][col])) {
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
        record_player_action(state);
        return true;
    }

    if (state->revealed[row][col] && state->board[row][col][0] != '\0' && is_player_piece(state, state->board[row][col])) {
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
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    int movable_from_rows[TEAMWORK_ROWS * TEAMWORK_COLS];
    int movable_from_cols[TEAMWORK_ROWS * TEAMWORK_COLS];
    int movable_count;
    int pick;
    int d;

    if (try_computer_capture_move(state)) {
        return true;
    }

    if (try_computer_evasion_move(state)) {
        return true;
    }

    movable_count = 0;

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (!state->revealed[row][col]) continue;
            if (state->board[row][col][0] == '\0') continue;
            if (!is_computer_piece(state, state->board[row][col])) continue;

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
                record_computer_action(state);
                return true;
            }
        }
    }

    return computer_flip(state);
}