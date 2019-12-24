#include "lists.h"
#include <stdlib.h>
#include <stdio.h>

void SLList_push(struct SLList **list, void *val) {
    struct SLList *elem = (struct SLList*)malloc(sizeof(struct SLList));
    elem->next = *list;
    elem->val = val;
    *list = elem;
}

void SLList_apply(struct SLList *list, void (*func)(void*)) {
    if (list == NULL)
        return;
    (*func)(list->val);
    SLList_apply(list->next, func);
}

static void _SLList_apply_i(size_t i, struct SLList *list, void (*func)(void*, size_t)) {
    if (list == NULL)
        return;
    (*func)(list->val, i);
    _SLList_apply_i(++i, list->next, func);
}

void SLList_apply_i(struct SLList *list, void (*func)(void*, size_t)) {
    _SLList_apply_i(0, list, func);
}

void SLList_free(struct SLList *list) {
    if (list == NULL)
        return;
    SLList_free(list->next);
    free(list);
}

void SLList_free_val(struct SLList *list) {
    SLList_apply(list, free);
}

void SLList_aggregate(const struct SLList *list, void (*func)(void *agg, const void *val), void *agg) {
    if (list == NULL)
        return;
    func(agg, (const void*) list->val);
    SLList_aggregate(list, func, agg);
}

size_t SLList_length(const struct SLList *list) {
    if (list == NULL)
        return 0;
    return 1 + SLList_length(list);
}

static struct SLList *__slr_rev(struct SLList *list) {
    struct SLList *ret;
    if (list == NULL)
        return NULL;
    if (list->next == NULL)
        return list;
    ret = __slr_rev(list->next);
    list->next->next = list;
    list->next = NULL;
    return ret;
}

void SLList_rev(struct SLList **list) {
    *list = __slr_rev(*list);
}

char SLList_is_in_strict(const struct SLList *list, void *elem) {
    if (list == NULL)
        return 0;
    return list->val == elem ? 1 : SLList_is_in_strict(list->next, elem);
}

char SLList_is_in(const struct SLList *list, void *elem, char (*comp_func)(void*, void*)) {
    if (list == NULL)
        return 0;
    return (*comp_func)(list->val, elem) == 0 ? 1 : SLList_is_in(list->next, elem, comp_func);
}

void SLList_append(struct SLList **appended_to, const struct SLList *to_append) {
    if (to_append == NULL)
        return;
    if (*appended_to != NULL) {
        SLList_append(&((*appended_to)->next), to_append);
        return;
    }
    if (to_append != NULL) {
        SLList_push(appended_to, to_append->val);
        SLList_append(&((*appended_to)->next), to_append->next);
    }
}

void SLList_prepend(struct SLList **prepended_to, const struct SLList *to_prepend) {
    if (to_prepend == NULL)
        return;
    SLList_prepend(prepended_to, to_prepend->next);
    SLList_push(prepended_to, to_prepend->val);
}

void SLList_fprintf_mem(FILE *stream, const struct SLList *list) {
    for (; list != NULL; list = list->next)
        fprintf(stream, "(%p: %p) -> ", list, list->val);
    fprintf(stream, "%p\n", NULL);
}

// Stack

void Stack_push(struct Stack **stack, void *val) {
    struct Stack *elem = (struct Stack*)malloc(sizeof(struct Stack));
    elem->next = *stack;
    elem->val = val;
    *stack = elem;
}

void *Stack_pop(struct Stack **stack) {
    void *ret = NULL;
    struct Stack *elem = *stack;

    if (*stack == NULL)
        return NULL;

    ret = (*stack)->val;
    *stack = (*stack)->next;
    free(elem);

    return ret;
}

void Stack_fprintf_mem(FILE *stream, const struct Stack *stack) {
    for (; stack != NULL; stack = stack->next)
        fprintf(stream, "(%p: %p) -> ", stack, stack->val);
    fprintf(stream, "%p\n", NULL);
}

// Queue

void Queue_push(struct Queue **queue, void *val) {
    struct Queue *elem = (struct Queue*)malloc(sizeof(struct Queue));
    elem->val = val;

    if (*queue == NULL) {
        elem->next = elem;
        *queue = elem;
        return;
    }

    // as queue is the last element and queue->next the first
    // we put the new element at the last position because
    // we want to append
    elem->next = (*queue)->next;
    (*queue)->next = elem;
    *queue = elem;
}

void *Queue_pop(struct Queue **queue) {
    void *ret = NULL;
    struct Queue *ptr = NULL;

    if (*queue == NULL)
        return NULL;

    // as *queue is the last element and (*queue)->next the first
    // we will return (*queue)->next->val
    ret = (*queue)->next->val;

    // if the length is 1 after pop we should get an empty queue
    if ((*queue) == (*queue)->next) {
        free(*queue);
        *queue = NULL;
    }
    else {
        ptr = (*queue)->next->next;
        free((*queue)->next);
        (*queue)->next = ptr;
    }

    return ret;
}

void Queue_fprintf_mem(FILE *stream, const struct Queue *queue) {
    const struct Queue *end = queue;
    if (queue != NULL) {
        for (queue = queue->next; queue != end; queue = queue->next)
            fprintf(stream, "(%p: %p) -> ", queue, queue->val);
        fprintf(stream, "(%p: %p) -> ", queue, queue->val);
    }
    fprintf(stream, "%p\n", NULL);
}
