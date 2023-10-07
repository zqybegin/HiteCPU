#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "log/ftrace.h"

void FuncList_add(FuncList *list, char *name, paddr_t start, paddr_t size){
    FuncNode *node = (FuncNode *)malloc(sizeof(FuncNode));
    strcpy(node->name, name);
    node->start = start;
    node->end = start + size;
    node->next = NULL;
    if (list->last == NULL){
        list->frist = node;
        list->last = node;
    } else {
        list->last->next = node;
        list->last = node;
    }
}

void FuncList_free(FuncList *list){
    FuncNode *prev = list->frist;
    FuncList_Foreach(list, frist->next, cur){
        free(prev);
        prev = cur;
    }
    free(prev);
    free(list);
}

char *FuncList_search(FuncList *list, paddr_t addr){
    FuncList_Foreach(list, frist, cur){
        if(addr >= cur->start && addr < cur->end){
            return cur->name;
        }
    }
    Assert(0, "func not found, addr is " FMT_WORD, addr);
}
