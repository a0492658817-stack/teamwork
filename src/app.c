#include "app.h"
#include <stdio.h>

int teamwork_prompt_turn_order(void) {
    int player_first;

    printf("Choose turn order:\n");
    printf("1. Player first\n");
    printf("2. Computer first\n");
    printf("Enter: ");
    fflush(stdout);

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

    if (player_first == 1) {
        state->player_camp = TEAMWORK_CAMP_RED;
        state->computer_camp = TEAMWORK_CAMP_BLACK;
    } else {
        state->player_camp = TEAMWORK_CAMP_BLACK;
        state->computer_camp = TEAMWORK_CAMP_RED;
    }

    ui->draw_board(ui->context, state);

    if (player_first == 2 && state->computer_moves < TEAMWORK_MAX_COMPUTER_MOVES) {
        ui->delay_ms(ui->context, 500);
        if (computer_move(state)) {
            printf("[LOG] Computer move #%d/%d completed.\n", state->computer_moves, TEAMWORK_MAX_COMPUTER_MOVES);
            ui->draw_board(ui->context, state);
        }
    }

        while (state->player_moves < TEAMWORK_MAX_PLAYER_MOVES ||
            state->computer_moves < TEAMWORK_MAX_COMPUTER_MOVES) {
        TeamworkUiEvent event;

        event = ui->poll_event(ui->context, &mouse_x, &mouse_y);

        if (event == TEAMWORK_UI_EVENT_QUIT) {
            break;
        }

        if (event == TEAMWORK_UI_EVENT_CLICK && player_select_or_move(state, mouse_x, mouse_y)) {
            printf("[LOG] Player move #%d/%d completed.\n", state->player_moves, TEAMWORK_MAX_PLAYER_MOVES);
            ui->draw_board(ui->context, state);

            if (state->computer_moves < TEAMWORK_MAX_COMPUTER_MOVES) {
                ui->delay_ms(ui->context, 500);
                if (computer_move(state)) {
                    printf("[LOG] Computer move #%d/%d completed.\n", state->computer_moves, TEAMWORK_MAX_COMPUTER_MOVES);
                    ui->draw_board(ui->context, state);
                }
            }
        }

        ui->delay_ms(ui->context, 30);
    }

    printf("\n[LOG] Game Over. Player=%d/%d, Computer=%d/%d, Total=%d\n",
           state->player_moves,
           TEAMWORK_MAX_PLAYER_MOVES,
           state->computer_moves,
           TEAMWORK_MAX_COMPUTER_MOVES,
           state->move_count);
}