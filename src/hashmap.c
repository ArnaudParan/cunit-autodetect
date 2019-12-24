#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_MAX_HASH (1 << 10)
#define DEFAULT_HASH_FUNCTION (HM_djb2)

unsigned int HM_djb2(const char *name) {
    unsigned long hash = 5381;
    unsigned int c;

    while ((c = (unsigned int) (*name++)) != 0)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

unsigned int HM_sdbm(const char *name) {
    unsigned long hash = 0;
    unsigned int c;

    while ((c = (unsigned int) (*name++)) != 0)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

unsigned int HM_lose_lose(const char *name) {
    unsigned int hash = 0;
    unsigned int c;

    while ((c = (unsigned int) (*name++)) != 0)
        hash += c;

    return hash;
}

void HM_init(struct HashMap *hm, unsigned int (*hash_function)(const char*), size_t max_hash) {
    if (hash_function != NULL) {
        hm->hash_function = hash_function;
        hm->n_buckets = max_hash;
        return;
    }
    hm->hash_function = DEFAULT_HASH_FUNCTION;
    hm->n_buckets = DEFAULT_MAX_HASH;
    hm->buckets = calloc(hm->n_buckets, sizeof(struct _hashmapelem*));

}

static void _hashmapelem_free(struct _hashmapelem *hme) {
    if (hme == NULL)
        return;
    _hashmapelem_free(hme->next);
    free(hme->name);
    free(hme);
}

void HM_clear(struct HashMap *hm) {
    int i;
    for (i = 0; i < hm->n_buckets; i++) {
        _hashmapelem_free(hm->buckets[i]);
        hm->buckets[i] = NULL;
    }
}

void HM_free(struct HashMap *hm) {
    HM_clear(hm);
    free(hm->buckets);
}

void HM_set(struct HashMap *hm, const char *name, void *val) {
    unsigned int hash = hm->hash_function(name) % hm->n_buckets;
    struct _hashmapelem *it = hm->buckets[hash];

    for (; it != NULL; it = it->next) {
        if (strcmp(name, it->name) == 0) {
            it->val = val;
            return;
        }
    }

    it = malloc(sizeof(struct _hashmapelem));
    it->name = calloc(strlen(name) + 1, sizeof(char));
    strcpy(it->name, name);
    it->val = val;
    it->next = hm->buckets[hash];
    hm->buckets[hash] = it;

}

void *HM_get(const struct HashMap *hm, const char *name) {
    unsigned int hash = hm->hash_function(name) % hm->n_buckets;
    struct _hashmapelem *it = hm->buckets[hash];

    for (; it != NULL; it = it->next) {
        if (strcmp(name, it->name) == 0)
            return it->val;
    }

    return NULL;
}

void HM_del(struct HashMap *hm, const char *name) {
    unsigned int hash = hm->hash_function(name) % hm->n_buckets;
    struct _hashmapelem *it = hm->buckets[hash], *it_prev = NULL;

    for (; it != NULL; it_prev = it, it = it->next) {
        if (strcmp(name, it->name) == 0) {
            if (it_prev == NULL)
                hm->buckets[hash] = it->next;
            else
                it_prev->next = it->next;
            free(it->name);
            free(it);
            return;
        }
    }
}

char HM_is_in(const struct HashMap *hm, const char *name) {
    unsigned int hash = hm->hash_function(name) % hm->n_buckets;
    struct _hashmapelem *it = hm->buckets[hash], *it_prev = NULL;

    for (; it != NULL; it_prev = it, it = it->next)
        if (strcmp(name, it->name) == 0)
            return 1;
    return 0;
}

void HM_fprintf_mem(FILE *stream, const struct HashMap *hm) {
    unsigned int i;
    const struct _hashmapelem *it;
    for (i = 0; i < hm->n_buckets; i++) {
        if (hm->buckets[i] != NULL)
            fprintf(stream, "%d : ", i);
        for (it = hm->buckets[i]; it != NULL; it = it->next)
            fprintf(stream, "(%p, \"%s\": %p) -> ", it, it->name, it->val);
        if (hm->buckets[i] != NULL)
            fprintf(stream, "%p\n", NULL);
    }
}

void HM_recompute(struct HashMap *hm, unsigned int (*hash_function)(const char*), size_t max_hash) {
    struct HashMap hmr;
    struct _hashmapelem *it = NULL;
    size_t i;

    HM_init(&hmr, hash_function, max_hash);

    for (i = 0; i < hm->n_buckets; i++) {
        for (it = hm->buckets[i]; it != NULL; it = it->next) {
            HM_set(&hmr, it->name, it->val);
        }
    }

    HM_free(hm);

    *hm = hmr;
}
