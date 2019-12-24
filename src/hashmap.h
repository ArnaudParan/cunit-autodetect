#ifndef DATA_STRUCTURES__HASHMAP_H
#define DATA_STRUCTURES__HASHMAP_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct  HashMap {
    struct _hashmapelem **buckets;
    size_t n_buckets;
    unsigned int (*hash_function)(const char*);
};

struct _hashmapelem {
    struct _hashmapelem *next;
    char *name;
    void *val;
};

unsigned int HM_djb2(const char *name);
unsigned int HM_sdbm(const char *name);
unsigned int HM_lose_lose(const char *name);

void HM_init(struct HashMap *hm, unsigned int (*hash_function)(const char*), size_t max_hash);
void HM_clear(struct HashMap *hm);
void HM_free(struct HashMap *hm);
void HM_set(struct HashMap *hm, const char *name, void *val);
void *HM_get(const struct HashMap *hm, const char *name);
void HM_del(struct HashMap *hm, const char *name);
char HM_is_in(const struct HashMap *hm, const char *name);
void HM_fprintf_mem(FILE *stream, const struct HashMap *hm);

#ifdef __cplusplus
}
#endif

#endif
