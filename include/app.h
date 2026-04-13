#ifndef TEAMWORK_APP_H
#define TEAMWORK_APP_H

#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TEAMWORK_UI_EVENT_NONE = 0,
    TEAMWORK_UI_EVENT_CLICK,
    TEAMWORK_UI_EVENT_QUIT
} TeamworkUiEvent;

typedef struct {
    void *context;
    TeamworkUiEvent (*poll_event)(void *context, int *mouse_x, int *mouse_y);
    void (*draw_board)(void *context, const GameState *state);
    void (*delay_ms)(void *context, unsigned int milliseconds);
} TeamworkUi;

int teamwork_prompt_turn_order(void);
void teamwork_run_game_session(GameState *state, const TeamworkUi *ui, int player_first);

#ifdef __cplusplus
}
#endif

#endif
