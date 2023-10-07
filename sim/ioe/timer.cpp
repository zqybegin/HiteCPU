#include <stdio.h>
#include <sys/time.h>

#include "map.h"
#include "debug.h"

// ----- get time of host -----
static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
    return us;
}

uint64_t get_time() {
    if (boot_time == 0)
        boot_time = get_time_internal();
    uint64_t now = get_time_internal();
    return now - boot_time;
}

// ----- IO timer device -----
static uint8_t *rtc_port_base = NULL;

static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
    Assert(offset == 0 || offset == 4, "offset is %d\n", offset);
    if (!is_write && offset == 0) {
        uint64_t us = get_time();
        rtc_port_base[0] = (uint32_t)us;
        rtc_port_base[1] = us >> 32;
    }
}

void init_timer() {
    rtc_port_base = (uint8_t *)new_space(8);

    add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
}
