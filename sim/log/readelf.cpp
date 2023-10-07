#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

#include "debug.h"
#include "log/ftrace.h"

char *read_section(FILE *fp, Elf32_Shdr sh) {
    char *buf = (char *)malloc(sh.sh_size);
    assert(buf != NULL);

    assert(fseek(fp, sh.sh_offset, SEEK_SET) == 0);
    assert(fread(buf, 1, sh.sh_size, fp) == sh.sh_size);

    return buf;
}

void init_elf(char *elf_file) {
    // allocate memory for func_list which storage func
    func_list = (FuncList *)malloc(sizeof(FuncList));
    // create file point
    FILE *fp = fopen(elf_file, "r");
    Assert(fp != NULL, "elf_file open fail");
    // read elf_header
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    Assert(elf_header != NULL, "elf header malloc fail");
    Assert(fread(elf_header, 1, sizeof(Elf32_Ehdr), fp) == sizeof(Elf32_Ehdr), "elf_header read fail");
    // record symbol_table section number
    size_t sym_number = 0;
    // read section_header
    Assert(fseek(fp, elf_header->e_shoff, SEEK_SET) == 0, "file point reset error");
    Elf32_Shdr *section_header = (Elf32_Shdr *)malloc(elf_header->e_shnum * sizeof(Elf32_Shdr));
    Assert(section_header != NULL, "section header malloc fail");
    for (size_t i = 0; i < elf_header->e_shnum; i++) {
        Assert(fread(section_header + i, 1, sizeof(Elf32_Shdr), fp) == sizeof(Elf32_Shdr), "section header %ld read fail", i);
        if (section_header[i].sh_type == SHT_SYMTAB) sym_number = i;
    }
    // read symbol_table
    Elf32_Sym *sym_tbl = (Elf32_Sym *)read_section(fp, section_header[sym_number]);
    size_t sym_count = section_header[sym_number].sh_size / sizeof(Elf32_Sym);
    size_t str_number = section_header[sym_number].sh_link;
    char *str_tab = read_section(fp, section_header[str_number]);
    // store symbol_table
    for (size_t i = 0; i < sym_count; i++) {
        if (ELF32_ST_TYPE(sym_tbl[i].st_info) == STT_FUNC) {
            FuncList_add(func_list, str_tab + sym_tbl[i].st_name, sym_tbl[i].st_value, sym_tbl[i].st_size);
        }
    }
    // free all malloc
    free(str_tab);
    free(sym_tbl);
    free(section_header);
    free(elf_header);
    fclose(fp);
}
