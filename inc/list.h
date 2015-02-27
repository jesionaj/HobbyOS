// 2015 Adam Jesionowski

/*
 * Doubly linked list.
 *
 * Note that List_ts are meant to be a members of a larger struct, which the owner
 * field points back to. This allows the list owner to know what task, e.g., the list
 * element is associated with.
 */

#ifndef LIST_H
#define	LIST_H

#include "config.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _list_t
{
    struct _list_t* prev;
    struct _list_t* next;
    void*           owner;  // List_t structs are implemented be a part of a larger data structure, which this points to
} List_t;

void AppendToList(List_t** root, List_t* node);
void AppendToEndOfList(List_t** root, List_t* node);
void RemoveFromList(List_t** root, List_t* node);
void RemoveFront(List_t** root);
bool IsNodeInList(List_t** root, List_t* node);

#ifdef	__cplusplus
}
#endif

#endif	/* LIST_H */

