#include "../include/game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
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

static void draw_board(const GameState *state) {
    cleardevice();

    setcolor(WHITE);
    outtextxy(50, 20, "Dark Chess");
    outtextxy(170, 20, "Player click one covered piece");

    for (int row = 0; row < TEAMWORK_ROWS; ++row) {
        for (int col = 0; col < TEAMWORK_COLS; ++col) {
            draw_one_cell(state, row, col);
        }
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    int playerFirst;
    printf("Choose turn order:\n");
    printf("1. Player first\n");
    printf("2. Computer first\n");
    printf("Enter: ");
    scanf("%d", &playerFirst);

    initwindow(TEAMWORK_WIN_W, TEAMWORK_WIN_H, "Dark Chess");
    setbkcolor(BLACK);
    cleardevice();

    GameState state;
    init_board(&state);
    draw_board(&state);

    if (playerFirst == 2) {
        delay(500);
        computer_flip(&state);
        draw_board(&state);
    }

    while (true) {
        if (all_revealed(&state)) {
            setcolor(YELLOW);
            outtextxy(50, 400, "All pieces are revealed.");
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mouseX;
            int mouseY;
            getmouseclick(WM_LBUTTONDOWN, mouseX, mouseY);

            if (player_flip_from_click(&state, mouseX, mouseY)) {
                draw_board(&state);

                if (!all_revealed(&state)) {
                    delay(500);
                    computer_flip(&state);
                    draw_board(&state);
                }
            }
        }

        delay(30);
    }

    getch();
    closegraph();
    return 0;
}
#else
int main(void) {
    srand((unsigned)time(NULL));

    GameState state;
    init_board(&state);

    printf("Dark Chess core initialized on a non-Windows platform.\n");
    printf("Board size: %d x %d\n", TEAMWORK_ROWS, TEAMWORK_COLS);
    printf("Assets root: %s\n", image_path_root());
    printf("All pieces revealed: %s\n", all_revealed(&state) ? "yes" : "no");

    return 0;
}
#endif
