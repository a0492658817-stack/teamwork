#include "app.h"

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

static TeamworkUiEvent poll_windows_event(void *context, int *mouse_x, int *mouse_y) {
    (void)context;

    if (ismouseclick(WM_LBUTTONDOWN)) {
        getmouseclick(WM_LBUTTONDOWN, *mouse_x, *mouse_y);
        return TEAMWORK_UI_EVENT_CLICK;
    }

    return TEAMWORK_UI_EVENT_NONE;
}

static void delay_windows(void *context, unsigned int milliseconds) {
    (void)context;

    delay((int)milliseconds);
}

static void draw_board(void *context, const GameState *state) {
    (void)context;

    cleardevice();

    setcolor(WHITE);
    outtextxy(50, 20, "Dark Chess");
    outtextxy(170, 20, "Player click one covered piece");

    for (int row = 0; row < TEAMWORK_ROWS; ++row) {
        for (int col = 0; col < TEAMWORK_COLS; ++col) {
            draw_one_cell(state, row, col);
        }
    }
    if (state->selected_row != -1) {
        int sx = TEAMWORK_START_X + state->selected_col * TEAMWORK_CELL;
        int sy = TEAMWORK_START_Y + state->selected_row * TEAMWORK_CELL;
        setcolor(YELLOW);
        setlinestyle(SOLID_LINE, 0, 3);
        rectangle(sx, sy, sx + TEAMWORK_CELL, sy + TEAMWORK_CELL);
        setlinestyle(SOLID_LINE, 0, 1);
    }
}

int teamwork_run_platform_app(void) {
    TeamworkUi ui;
    GameState state;
    int player_first;

    player_first = teamwork_prompt_turn_order();

    initwindow(TEAMWORK_WIN_W, TEAMWORK_WIN_H, "Dark Chess");
    setbkcolor(BLACK);
    cleardevice();

    init_board(&state);

    ui.context = NULL;
    ui.poll_event = poll_windows_event;
    ui.draw_board = draw_board;
    ui.delay_ms = delay_windows;

    teamwork_run_game_session(&state, &ui, player_first);
    closegraph();
    return 0;
}