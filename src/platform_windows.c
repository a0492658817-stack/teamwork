#include "app.h"
#include "platform.h"

#include <conio.h>
#include <graphics.h>
#include <stdio.h>

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

static int prompt_turn_order_click_windows(void) {
    int player_x1 = 140;
    int player_y1 = 180;
    int player_x2 = 360;
    int player_y2 = 270;
    int computer_x1 = 440;
    int computer_y1 = 180;
    int computer_x2 = 660;
    int computer_y2 = 270;

    printf("[LOG] Click left box for Player First, right box for Computer First.\n");
    fflush(stdout);

    while (1) {
        int x;
        int y;

        cleardevice();
        setcolor(WHITE);
        outtextxy(180, 120, "Select turn order by clicking a box");

        setfillstyle(SOLID_FILL, GREEN);
        bar(player_x1, player_y1, player_x2, player_y2);
        setcolor(WHITE);
        rectangle(player_x1, player_y1, player_x2, player_y2);
        outtextxy(player_x1 + 60, player_y1 + 35, "玩家先手");

        setfillstyle(SOLID_FILL, RED);
        bar(computer_x1, computer_y1, computer_x2, computer_y2);
        setcolor(WHITE);
        rectangle(computer_x1, computer_y1, computer_x2, computer_y2);
        outtextxy(computer_x1 + 60, computer_y1 + 35, "電腦先手");

        if (ismouseclick(WM_LBUTTONDOWN)) {
            getmouseclick(WM_LBUTTONDOWN, x, y);

            if (x >= player_x1 && x <= player_x2 && y >= player_y1 && y <= player_y2) {
                printf("[LOG] Turn order selected: Player first.\n");
                fflush(stdout);
                return 1;
            }
            if (x >= computer_x1 && x <= computer_x2 && y >= computer_y1 && y <= computer_y2) {
                printf("[LOG] Turn order selected: Computer first.\n");
                fflush(stdout);
                return 2;
            }
        }

        delay(16);
    }
}

int teamwork_run_platform_app(void) {
    TeamworkUi ui;
    GameState state;
    int player_first;
    int window_id;
    int graph_error;

    printf("[teamwork] starting WinBGIm...\n");
    fflush(stdout);

    window_id = initwindow(TEAMWORK_WIN_W, TEAMWORK_WIN_H, "Dark Chess");
    graph_error = graphresult();
    if (window_id < 0 || graph_error != grOk) {
        fprintf(stderr, "[teamwork] WinBGIm init failed: window_id=%d, graph_error=%d (%s)\n",
                window_id,
                graph_error,
                grapherrormsg(graph_error));
        fflush(stderr);
        return 1;
    }

    printf("[teamwork] window created (id=%d).\n", window_id);
    fflush(stdout);

    setbkcolor(BLACK);
    cleardevice();

    init_board(&state);
    player_first = prompt_turn_order_click_windows();

    ui.context = NULL;
    ui.poll_event = poll_windows_event;
    ui.draw_board = draw_board;
    ui.delay_ms = delay_windows;

    teamwork_run_game_session(&state, &ui, player_first);
    closegraph();
    return 0;
}