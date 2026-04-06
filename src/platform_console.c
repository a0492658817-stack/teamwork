#include "app.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int teamwork_run_platform_app(void) {
    GameState state;

    init_board(&state);

    printf("Dark Chess core initialized on a non-Windows platform.\n");
    printf("Board size: %d x %d\n", TEAMWORK_ROWS, TEAMWORK_COLS);
    printf("Assets root: %s\n", image_path_root());
    printf("All pieces revealed: %s\n", all_revealed(&state) ? "yes" : "no");

    return 0;
}