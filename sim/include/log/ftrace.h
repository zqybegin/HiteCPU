#include "common.h"

typedef struct FuncNode {
    char name[128];
    paddr_t start;
    paddr_t end;
    struct FuncNode *next;
} FuncNode;

typedef struct FuncList {
    struct FuncNode *frist;
    struct FuncNode *last;
} FuncList;

extern FuncList *func_list;
extern FILE *ftrace_fp;

void FuncList_add(FuncList *list, char *name, paddr_t start, paddr_t size);
void FuncList_free(FuncList *list);
char *FuncList_search(FuncList *list, paddr_t addr);

#define FuncList_Foreach(L, S, V) \
    FuncNode *_node = NULL;       \
    FuncNode *V = NULL;           \
    for (V = _node = L->S; _node != NULL; V = _node = _node->next)
