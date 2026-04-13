#include "app.h"
#include <stdio.h>

int teamwork_prompt_turn_order(void) {
    int player_first;

    printf("Choose turn order:\n");
    printf("1. Player first\n");
    printf("2. Computer first\n");
    printf("Enter: ");

    if (scanf("%d", &player_first) != 1) {
        player_first = 1;
    }

    if (player_first != 1 && player_first != 2) {
        player_first = 1;
    }

    return player_first;
}

void teamwork_run_game_session(GameState *state, const TeamworkUi *ui, int player_first) {
    int mouse_x;
    int mouse_y;

    if (ui == NULL || ui->draw_board == NULL || ui->delay_ms == NULL || ui->poll_event == NULL) {
        return;
    }

    ui->draw_board(ui->context, state);

    if (player_first == 2) {
        ui->delay_ms(ui->context, 500);
        computer_move(state);
        ui->draw_board(ui->context, state);
    }

    /* 第六步：修改迴圈判斷式，增加 state->move_count < 20 的條件 */
    while (!all_revealed(state) && state->move_count < 20) {
        TeamworkUiEvent event;

        event = ui->poll_event(ui->context, &mouse_x, &mouse_y);

        if (event == TEAMWORK_UI_EVENT_QUIT) {
            break;
        }

        if (event == TEAMWORK_UI_EVENT_CLICK && player_select_or_move(state, mouse_x, mouse_y)) {
            ui->draw_board(ui->context, state);

            /* 電腦行動前也要檢查步數是否已滿 */
            if (!all_revealed(state) && state->move_count < 20) {
                ui->delay_ms(ui->context, 500);
                computer_move(state);
                ui->draw_board(ui->context, state);
            }
        }

        ui->delay_ms(ui->context, 30);
    }

    /* 遊戲結束後的提示（可選） */
    if (state->move_count >= 20) {
        printf("\nGame Over: Reached maximum moves (20).\n");
    }
}