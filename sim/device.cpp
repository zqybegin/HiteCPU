#include "map.h"
#include "debug.h"

void init_serial();
void init_timer();

void init_device() {
    init_map();

    printf(ANSI_FMT("Device IO: ON\n", ANSI_FG_BLUE));
    init_serial();
    init_timer();
}
