#include <cstdio>
#include <getopt.h>

#include "common.h"

// initial file variable
char *img_file;
char *vcd_file;
char *diff_so_file;

int parse_args(int argc, char *argv[]) {
    const struct option table[] = {
        {"diff", required_argument, NULL, 'd'},
        {"vcd",  required_argument, NULL, 'v'},
        {"img",  required_argument, NULL, 'i'},
        {0,      0,                 NULL,  0 },
    };

    int o;
    while ((o = getopt_long(argc, argv, "hd:v:i:", table, NULL)) != -1) {
        switch (o) {
            case 'd':
                diff_so_file= optarg;
                break;
            case 'v':
                vcd_file = optarg;
                break;
            case 'i':
                img_file = optarg;
                break;
            default:
                printf("Usage: %s [OPTION...] [args]\n\n", argv[0]);
                printf("\t-v,--vcd=FILE.vcd       set vcd file generated path" ANSI_FMT("(required)", ANSI_FG_BLUE) "\n");
                printf("\t-d,--diff=REF_SO        set reference REF_SO" ANSI_FMT("(required)", ANSI_FG_BLUE) "\n");
                printf("\t-e,--img=FILE.bin       set img file\n");
                printf("\n");
                return -1;
            }
    }

    if (diff_so_file == NULL) {
        printf(ANSI_FMT("simulate argument error: ", ANSI_FG_BLUE) "you must set reference REF_SO\n");
        return -1;
    }
    if (vcd_file == NULL) {
        printf(ANSI_FMT("simulate argument error: ", ANSI_FG_BLUE) "you must set VCD file generated path\n");
        return -1;
    }

    return 0;
}
