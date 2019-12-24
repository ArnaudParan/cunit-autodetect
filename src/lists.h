#ifndef DATA_STRUCTURES_LISTS_H
#define DATA_STRUCTURES_LISTS_H
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Singly linked list
struct SLList {
    void *val;
    struct SLList *next;
};

void SLList_push(struct SLList **list, void *val);
void SLList_apply(struct SLList *list, void (*func)(void*));
void SLList_apply_i(struct SLList *list, void (*func)(void*, size_t));
void SLList_free(struct SLList *list);
void SLList_free_val(struct SLList *list);
void SLList_aggregate(const struct SLList *list, void (*func)(void *agg, const void *val), void *agg);
size_t SLList_length(const struct SLList *list);
void SLList_rev(struct SLList **list);
// compares the pointers and tells if the value is in the list
char SLList_is_in_strict(const struct SLList *list, void *elem);
// tells if the element is in the list with a compare function which should return 0 if
// the elements are equal
char SLList_is_in(const struct SLList *list, void *elem, char (*comp_func)(void*, void*));
void SLList_append(struct SLList **appended_to, const struct SLList *to_append);
void SLList_prepend(struct SLList **prepended_to, const struct SLList *to_prepend);
void SLList_fprintf_mem(FILE *stream, const struct SLList *list);


// Stack
struct Stack {
    void *val;
    struct Stack *next;
};

void Stack_push(struct Stack **stack, void *val);
void *Stack_pop(struct Stack **stack);
void Stack_fprintf_mem(FILE *stream, const struct Stack *stack);

// TODO cllist

// Queue
// Our queues are just circular singly linked lists, we reference their last element
// so that we can improve the algorithms and not have to go through the whole list
// at every pop
struct Queue {
    void *val;
    struct Queue *next;
};

void Queue_push(struct Queue **queue, void *val);
void *Queue_pop(struct Queue **queue);
void Queue_fprintf_mem(FILE *stream, const struct Queue *queue);

// TODO dllist
// TODO cdllist

// TODO skip list

#ifdef __cplusplus
}
#endif
#endif
