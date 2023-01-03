#ifndef __SET_H__
#define __SET_H__

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct {
    char *key;
    int keylen;
    int val;
} HashEntry;

typedef struct {
    HashEntry *buckets;
    int num_ents;
    int num_used_ents;
    int num_toks;
} HashMap;

int hashmap_get(HashMap *map, char *key);
void hashmap_put(HashMap *map, char *key, int keylen);
void hashmap_put2(HashMap *map, char *key, int keylen, int val);
void hashmap_test(void);

void print_maps(HashMap maps[]);

#endif /* __SET_H__ */
