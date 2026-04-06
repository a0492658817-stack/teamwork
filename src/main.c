#include "platform.h"

#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned)time(NULL));
    return teamwork_run_platform_app();
}
