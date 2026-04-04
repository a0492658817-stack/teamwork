#include "game.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool has_duplicate_name(char names[][TEAMWORK_PIECE_NAME_LENGTH], int count, const char *candidate) {
    int index;
    for (index = 0; index < count; ++index) {
        if (strcmp(names[index], candidate) == 0) {
            return true;
        }
    }
    return false;
}

static int count_revealed(const GameState *state) {
    int row;
    int col;
    int count;

    count = 0;
    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            if (state->revealed[row][col]) {
                ++count;
            }
        }
    }
    return count;
}

static void test_initialization(void) {
    GameState state;
    char unique_names[32][TEAMWORK_PIECE_NAME_LENGTH];
    int unique_count;
    int row;
    int col;

    srand(1234);
    init_board(&state);

    unique_count = 0;
    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            assert(!state.revealed[row][col]);
            assert(strlen(state.board[row][col]) > 0);

            if (!has_duplicate_name(unique_names, unique_count, state.board[row][col])) {
                strcpy(unique_names[unique_count], state.board[row][col]);
                ++unique_count;
            }
        }
    }

    assert(unique_count > 0);
}

static void test_board_math(void) {
    int row;
    int col;

    row = -1;
    col = -1;

    click_to_cell(TEAMWORK_START_X + TEAMWORK_CELL * 3 + 10,
                  TEAMWORK_START_Y + TEAMWORK_CELL * 1 + 10,
                  &row,
                  &col);

    assert(row == 1);
    assert(col == 3);

    click_to_cell(TEAMWORK_START_X - 1, TEAMWORK_START_Y - 1, &row, &col);
    assert(row == -1);
    assert(col == -1);

    assert(in_board(0, 0));
    assert(!in_board(-1, 0));
    assert(!in_board(0, TEAMWORK_COLS));
}

static void test_image_path_helpers(void) {
    char full_path[128];
    char tiny_path[8];

    assert(strcmp(image_path_root(), "data/images") == 0);

    compose_image_path("redmaster.bmp", full_path, sizeof(full_path));
    assert(strstr(full_path, "data/images/") == full_path);
    assert(strstr(full_path, "redmaster.bmp") != NULL);

    compose_image_path("redmaster.bmp", tiny_path, sizeof(tiny_path));
    assert(tiny_path[sizeof(tiny_path) - 1] == '\0');
}

static void test_reveal_flow(void) {
    GameState state;
    int row;
    int col;

    srand(1);
    init_board(&state);

    assert(!all_revealed(&state));
    assert(computer_flip(&state));
    assert(!all_revealed(&state));

    for (row = 0; row < TEAMWORK_ROWS; ++row) {
        for (col = 0; col < TEAMWORK_COLS; ++col) {
            state.revealed[row][col] = true;
        }
    }

    assert(all_revealed(&state));
    assert(!computer_flip(&state));
}

static void test_player_invalid_click_no_flip(void) {
    GameState state;

    srand(2);
    init_board(&state);

    assert(!player_flip_from_click(&state, TEAMWORK_START_X - 1, TEAMWORK_START_Y - 1));
    assert(count_revealed(&state) == 0);
}

static void test_player_already_revealed_click_no_flip(void) {
    GameState state;
    int click_x;
    int click_y;

    srand(3);
    init_board(&state);

    click_x = TEAMWORK_START_X + 10;
    click_y = TEAMWORK_START_Y + 10;

    assert(player_flip_from_click(&state, click_x, click_y));
    assert(count_revealed(&state) == 1);
    assert(!player_flip_from_click(&state, click_x, click_y));
    assert(count_revealed(&state) == 1);
}

static void test_player_then_computer_single_reveal_each(void) {
    GameState state;

    srand(4);
    init_board(&state);

    assert(player_flip_from_click(&state, TEAMWORK_START_X + 20, TEAMWORK_START_Y + 20));
    assert(count_revealed(&state) == 1);

    assert(computer_flip(&state));
    assert(count_revealed(&state) == 2);
}

int main(void) {
    test_initialization();
    test_board_math();
    test_image_path_helpers();
    test_reveal_flow();
    test_player_invalid_click_no_flip();
    test_player_already_revealed_click_no_flip();
    test_player_then_computer_single_reveal_each();
    return 0;
}