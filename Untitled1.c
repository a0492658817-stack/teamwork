#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

const int ROWS = 4;
const int COLS = 8;
const int CELL = 80;
const int START_X = 50;
const int START_Y = 50;
const int WIN_W = 800;
const int WIN_H = 500;

char board[ROWS][COLS][40];
bool revealed[ROWS][COLS];

// 32 顆暗棋，依你的圖片檔名
const char *pieces[32] = {
    // red 16
    "redmaster.bmp",
    "redknight.bmp", "redknight.bmp",
    "redelephant.bmp", "redelephant.bmp",
    "redcar.bmp", "redcar.bmp",
    "redhorse.bmp", "redhorse.bmp",
    "redcannon.bmp", "redcannon.bmp",
    "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp", "redsoldier.bmp",

    // black 16
    "blackmaster.bmp",
    "blackknight.bmp", "blackknight.bmp",
    "blackelephant.bmp", "blackelephant.bmp",
    "blackcar.bmp", "blackcar.bmp",
    "blackhorse.bmp", "blackhorse.bmp",
    "blackcannon.bmp", "blackcannon.bmp",
    "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp", "blacksoldier.bmp"
};

void initBoard() {
    int idx[32];
    for (int i = 0; i < 32; i++) idx[i] = i;

    // 洗牌
    for (int i = 0; i < 32; i++) {
        int j = rand() % 32;
        int t = idx[i];
        idx[i] = idx[j];
        idx[j] = t;
    }

    int k = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            strcpy(board[r][c], pieces[idx[k++]]);
            revealed[r][c] = false;
        }
    }
}

void drawOneCell(int r, int c) {
    int x1 = START_X + c * CELL;
    int y1 = START_Y + r * CELL;
    int x2 = x1 + CELL;
    int y2 = y1 + CELL;

    // 格線
    setcolor(WHITE);
    rectangle(x1, y1, x2, y2);

    // 圖片顯示區域，留點邊距
    int imgL = x1 + 5;
    int imgT = y1 + 5;
    int imgR = x2 - 5;
    int imgB = y2 - 5;

    if (!revealed[r][c]) {
        // 蓋牌
        readimagefile("background.bmp", imgL, imgT, imgR, imgB);
    } else {
        // 翻開棋子
        readimagefile(board[r][c], imgL, imgT, imgR, imgB);
    }
}

void drawBoard() {
    cleardevice();

    setcolor(WHITE);
    outtextxy(50, 20, "Dark Chess");
    outtextxy(170, 20, "Player click one covered piece");

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            drawOneCell(r, c);
        }
    }
}

void clickToCell(int mx, int my, int &r, int &c) {
    c = (mx - START_X) / CELL;
    r = (my - START_Y) / CELL;
}

bool inBoard(int r, int c) {
    return r >= 0 && r < ROWS && c >= 0 && c < COLS;
}

bool allRevealed() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!revealed[r][c]) return false;
        }
    }
    return true;
}

void computerFlip() {
    int listR[32], listC[32];
    int cnt = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!revealed[r][c]) {
                listR[cnt] = r;
                listC[cnt] = c;
                cnt++;
            }
        }
    }

    if (cnt > 0) {
        int pick = rand() % cnt;
        int rr = listR[pick];
        int cc = listC[pick];
        revealed[rr][cc] = true;
    }
}

int main() {
    srand((unsigned)time(NULL));

    initwindow(WIN_W, WIN_H, "Dark Chess");
    setbkcolor(BLACK);
    cleardevice();

    initBoard();
    drawBoard();

    while (true) {
        if (allRevealed()) {
            setcolor(YELLOW);
            outtextxy(50, 400, "All pieces are revealed.");
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);

            int r, c;
            clickToCell(mx, my, r, c);

            if (inBoard(r, c) && !revealed[r][c]) {
                // 玩家翻
                revealed[r][c] = true;
                drawBoard();

                // 電腦翻
                if (!allRevealed()) {
                    delay(500);
                    computerFlip();
                    drawBoard();
                }
            }
        }

        delay(30);
    }

    getch();
    closegraph();
    return 0;
}
