#include <cstdio>
#include <getopt.h>

#include "common.h"
#include "debug.h"

extern char *img_file, *diff_so_file, *vcd_file, *log_dir, *elf_file;

int parse_args(int argc, char *argv[]) {
    const struct option table[] = {
        {"diff", required_argument, NULL, 'd'},
        { "vcd", required_argument, NULL, 'v'},
        { "img", required_argument, NULL, 'i'},
        { "log", required_argument, NULL, 'l'},
        { "elf", required_argument, NULL, 'e'},
        {     0,                 0, NULL,   0},
    };

    int o;
    while ((o = getopt_long(argc, argv, "hd:v:i:", table, NULL)) != -1) {
        switch (o) {
            case 'd': diff_so_file = optarg; break;
            case 'v': vcd_file = optarg; break;
            case 'i': img_file = optarg; break;
            case 'l': log_dir  = optarg; break;
            case 'e': elf_file = optarg; break;
            default:
                printf("Usage: %s [OPTION...] [args]\n\n", argv[0]);
                printf("\t-v,--vcd=FILE.vcd    vcd file generated path" ANSI_FMT("(required)", ANSI_FG_RED) "\n");
                printf("\t-d,--diff=REF_SO     reference REF_SO" ANSI_FMT("(required)", ANSI_FG_RED) "\n");
                printf("\t-i,--img=FILE.bin    img file\n");
                printf("\t-l,--log=Log_Dir     the directory of log file\n");
                printf("\t-e,--elf=FILE.elf    elf file\n");
                printf("\n");
                return -1;
        }
    }

    IFDEF(CONFIG_DIFFTEST, Assert(diff_so_file,ANSI_FMT("simulate argument error: ", ANSI_FG_RED) "you must set reference REF_SO\n"));
    Assert(vcd_file,    ANSI_FMT("simulate argument error: ", ANSI_FG_RED) "you must set VCD file generated path\n");
    IFDEF(CONFIG_FTRACE, Assert(log_dir,     ANSI_FMT("simulate argument error: ", ANSI_FG_RED) "you must set the directory of log file\n"));
    IFDEF(CONFIG_FTRACE, Assert(elf_file,    ANSI_FMT("simulate argument error: ", ANSI_FG_RED) "you must set elf file\n"));

    return 0;
}
