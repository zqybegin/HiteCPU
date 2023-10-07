#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

#include "common.h"
#include "debug.h"
#include "log/ftrace.h"

FuncList *func_list;
FILE *ftrace_fp;

extern void init_elf(char *elf_file);

char *strcat_path(char *dir, const char *file){
    char *path = (char *)malloc(strlen(dir) + strlen(file) + 1);
    path = strcpy(path, dir);
    path = strcat(path, file);
    return path;
}

void init_log(char *log_dir, char *elf_file){

#ifdef CONFIG_FTRACE
    init_elf(elf_file);
    char *ftrace_path = strcat_path(log_dir, "/npc-func.txt");
    ftrace_fp = fopen(ftrace_path, "w");
    Assert(ftrace_fp , "Can not open '%s'", ftrace_path);

    printf(ANSI_FMT("Ftrace log: ON\n", ANSI_FG_BLUE));
    printf("The ftrace log file is in %s\n", ftrace_path);
    printf("The elf file file is in %s\n", elf_file);

    free(ftrace_path);
#endif

}

void close_log(){

#ifdef CONFIG_FTRACE
    fclose(ftrace_fp);
    FuncList_free(func_list);
#endif

}
