#include <cstdio>
#include <string.h>

#include "common.h"
#include "map.h"
#include "memory.h"

uint8_t pmem[CONFIG_MSIZE] = {};
static const uint32_t img[] = {
    0x00100113,
    0x00110113,
    0x00110113,
    0x00110113,
    0x00100073,
    // 0000000 00001 00010 000 00010 00100 11
    // 0000 0000 0001 0001 0000 0001 0001 0011
    // 0    0    1    1    0    1    1    3
};

uint8_t *guest_to_host(paddr_t paddr) {
    return pmem + paddr - CONFIG_MBASE;
}

word_t mem_read(paddr_t addr, int len) {
    if (in_pmem(addr)) {
        return host_read(guest_to_host(addr), len);
    } else {
        MUXDEF(CONFIG_DEVICE, return mmio_read(addr, len), Assert(0,"No Device but read addr in mmio\n"));
    }
}

void mem_write(paddr_t addr, int len, word_t data) {
    if (in_pmem(addr)) {
        host_write(guest_to_host(addr), len, data);
        return;
    } else {
        MUXDEF(CONFIG_DEVICE, mmio_write(addr, len, data), Assert(0,"No Device but write addr in mmio\n"));
    }
}

long mem_init(char *img_file) {
    if (img_file == NULL) {
        printf(ANSI_FMT("No image is given. Use the default build-in image.\n", ANSI_FG_BLUE));
        memcpy(guest_to_host(CONFIG_MBASE), img, sizeof(img));
        return sizeof(img);
    }

    FILE *fp = fopen(img_file, "rb");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    printf(ANSI_FMT("Use the specified image\n", ANSI_FG_YELLOW));
    printf("The image is %s, size = %ld\n", img_file, size);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(CONFIG_MBASE), size, 1, fp);
    assert(ret == 1);

    fclose(fp);
    return size;
}
