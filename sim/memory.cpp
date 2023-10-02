#include "common.h"
#include <cstdio>

uint8_t pmem[MEM_SIZE] = {};
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
    return pmem + paddr - START_ENTRY;
}

word_t mem_read(paddr_t addr, int len) {
    switch (len) {
        case 0: return *(uint8_t *)guest_to_host(addr);
        case 1: return *(uint16_t *)guest_to_host(addr);
        case 2: return *(uint32_t *)guest_to_host(addr);
        default: {
            printf("mem access len error, now is %d\n", len);
            assert(0);
        }
    }
}

void mem_write(paddr_t addr, int len, word_t data) {
    switch (len) {
        case 0: *(uint8_t *)guest_to_host(addr) = data; return;
        case 1: *(uint16_t *)guest_to_host(addr) = data; return;
        case 2: *(uint32_t *)guest_to_host(addr) = data; return;
        default: {
            printf("mem access len error, now is %d\n", len);
            assert(0);
        }
    }
}

long mem_init(char *img_file) {
    if (img_file == NULL) {
        printf(ANSI_FMT("No image is given. Use the default build-in image.\n", ANSI_FG_YELLOW));
        memcpy(guest_to_host(START_ENTRY), img, sizeof(img));
        return -1;
    }

    FILE *fp = fopen(img_file, "rb");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    printf("The image is %s, size = %ld\n", img_file, size);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(START_ENTRY), size, 1, fp);
    assert(ret == 1);

    fclose(fp);
    return size;
}
