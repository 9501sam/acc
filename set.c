#include "set.h"

// Initial hash bucket size
#define INIT_SIZE 16

// Rehash if the usage exceeds 70%.
#define HIGH_WATERMARK 70

// We'll keep the usage below 50% after rehashing.
#define LOW_WATERMARK 50

#define UNUSED (-1)

static uint64_t fnv_hash(char *s, int len) {
    uint64_t hash = 0xcbf29ce484222325;
    for (int i = 0; i < len; i++) {
        hash *= 0x100000001b3;
        hash ^= (unsigned char)s[i];
    }
    return hash;
}

static void rehash(HashMap *map) {
    // Compute the size of the new hashmap.
    int nkeys = 0;
    for (int i = 0; i < map->capacity; i++)
        if (map->buckets[i].key)
            nkeys++;

    int cap = map->capacity;
    while ((nkeys * 100) / cap >= LOW_WATERMARK)
        cap = cap * 2;
    assert(cap > 0);

    // Create a new hashmap and copy all key-values.
    HashMap map2 = {};
    map2.buckets = calloc(cap, sizeof(HashEntry));
    map2.capacity = cap;
    map2.num_toks = map->num_toks;

    for (int i = 0; i < map->capacity; i++) {
        HashEntry *ent = &map->buckets[i];
        if (ent->key)
            hashmap_put2(&map2, ent->key, ent->keylen, ent->val);
    }

    assert(map2.used == nkeys);
    *map = map2;
}

static void unreachable()
{
    fprintf(stderr, "error unreachable\n");
}

static bool match(HashEntry *ent, char *key, int keylen) {
    return ent->key && ent->keylen == keylen && 
        memcmp(ent->key, key, keylen) == 0;
}

static HashEntry *get_entry(HashMap *map, char *key, int keylen) {
    if (!map->buckets)
        return NULL;

    uint64_t hash = fnv_hash(key, keylen);

    for (int i = 0; i < map->capacity; i++) {
        HashEntry *ent = &map->buckets[(hash + i) % map->capacity];
        if (match(ent, key, keylen))
            return ent;
        if (ent->key == NULL)
            return NULL;
    }
    unreachable();
}

static HashEntry *get_or_insert_entry(HashMap *map, char *key, int keylen) {
    if (!map->buckets) {
        map->buckets = calloc(INIT_SIZE, sizeof(HashEntry));
        map->capacity = INIT_SIZE;
        map->num_toks = 0;
    } else if ((map->used * 100) / map->capacity >= HIGH_WATERMARK) {
        rehash(map);
    }

    uint64_t hash = fnv_hash(key, keylen);

    for (int i = 0; i < map->capacity; i++) {
        HashEntry *ent = &map->buckets[(hash + i) % map->capacity];

        if (match(ent, key, keylen))
            return ent;

        if (ent->key == NULL) {
            ent->key = key;
            ent->keylen = keylen;
            ent->val = 0;
            map->used++;
            return ent;
        }
    }
    unreachable();
}

int hashmap_get(HashMap *map, char *key)
{
    HashEntry *ent = get_entry(map, key, strlen(key));
    return ent ? ent->val : UNUSED;
}

void hashmap_put(HashMap *map, char *key)
{
    HashEntry *ent = get_or_insert_entry(map, key, strlen(key));
    (ent->val)++;
    (map->num_toks)++;
}

void hashmap_put2(HashMap *map, char *key, int keylen, int val)
{
    HashEntry *ent = get_or_insert_entry(map, key, keylen);
    ent->val = val;
}

///*** test ***///
static char *format(char *fmt, ...)
{
    char *buf;
    size_t buflen;
    FILE *out = open_memstream(&buf, &buflen);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fclose(out);
    return buf;
}

void hashmap_test(void)
{
    printf("======test hashmap======\n");

    HashMap *map = calloc(1, sizeof(HashMap));

    for (int i = 0; i < 5000; i++) {
        hashmap_put(map, format("key %d", i));
        assert(map->num_toks == (i + 1));
    }
    for (int i = 0; i < 5000; i++)
        assert(hashmap_get(map, format("key %d", i)) == 1);
    for (int i = 4000; i < 5000; i++)
        hashmap_put(map, format("key %d", i));
    for (int i = 0; i < 4000; i++)
        assert(hashmap_get(map, format("key %d", i)) == 1);
    for (int i = 4000; i < 5000; i++)
        assert(hashmap_get(map, format("key %d", i)) == 2);
    assert(map->used == 5000);
    assert(map->num_toks == 6000);

    // <stdio.h>
    // int ia, *ie, forif;
    // char ca;
    // ia = (*ie) + ib; ia = ia * 2;
    // ca = 'A'
    // whilefor (ia<3) forif = forif + 1;
    // sCAnf("%c", &cb);
    // ia = *forif + (-2);
    // ia = 5 * ie;
    HashMap *map2_res = calloc(1, sizeof(HashMap));
    HashMap *map2_lib = calloc(1, sizeof(HashMap));
    HashMap *map2_id = calloc(1, sizeof(HashMap));
    HashMap *map2_char = calloc(1, sizeof(HashMap));
    HashMap *map2_num = calloc(1, sizeof(HashMap));
    HashMap *map2_ptr = calloc(1, sizeof(HashMap));
    HashMap *map2_bkt = calloc(1, sizeof(HashMap));
    HashMap *map2_op = calloc(1, sizeof(HashMap));
    HashMap *map2_cmp = calloc(1, sizeof(HashMap));
    HashMap *map2_punc = calloc(1, sizeof(HashMap));
    HashMap *map2_fmt = calloc(1, sizeof(HashMap));
    HashMap *map2_prtd = calloc(1, sizeof(HashMap));
    HashMap *map2_cmmt = calloc(1, sizeof(HashMap));
    HashMap *map2_undef = calloc(1, sizeof(HashMap));
    HashMap *map2_skip = calloc(1, sizeof(HashMap));

    hashmap_put(map2_cmp, "<");
    hashmap_put(map2_undef, "stdio.h");
    hashmap_put(map2_skip, ">");
    hashmap_put(map2_res, "int");
    hashmap_put(map2_id, "ia");
    hashmap_put(map2_punc, ",");
    hashmap_put(map2_ptr, "*ie");
    hashmap_put(map2_punc, ",");
    hashmap_put(map2_id, "forif");
    hashmap_put(map2_punc, ";");
    hashmap_put(map2_res, "char");
    hashmap_put(map2_id, "ca");
    hashmap_put(map2_punc, ";");
    hashmap_put(map2_id, "ia");
    hashmap_put(map2_op, "=");
    hashmap_put(map2_bkt, "(");
    hashmap_put(map2_ptr, "*ie");
    hashmap_put(map2_bkt, ")");
    hashmap_put(map2_op, "+");
    hashmap_put(map2_undef, "ib");
    hashmap_put(map2_skip, "; ia = ia *2;");
    hashmap_put(map2_id, "ca");
    hashmap_put(map2_op, "=");
    hashmap_put(map2_char, "'A'");
    hashmap_put(map2_punc, ";");
    hashmap_put(map2_undef, "whilefor");
    hashmap_put(map2_skip, "(ia<3) forif = forif + 1;");
    hashmap_put(map2_res, "sCAnf");
    hashmap_put(map2_bkt, "(");
    hashmap_put(map2_punc, "\"");
    hashmap_put(map2_fmt, "\%c");
    hashmap_put(map2_punc, "\"");
    hashmap_put(map2_punc, ",");
    hashmap_put(map2_undef, "&cb");
    hashmap_put(map2_skip, ");");
    hashmap_put(map2_id, "ia");
    hashmap_put(map2_op, "=");
    hashmap_put(map2_op, "*");
    hashmap_put(map2_id, "forif");
    hashmap_put(map2_op, "+");
    hashmap_put(map2_num, "(-2)");
    hashmap_put(map2_punc, ";");
    hashmap_put(map2_id, "ia");
    hashmap_put(map2_op, "=");
    hashmap_put(map2_num, "5");
    hashmap_put(map2_ptr, "*ie");
    hashmap_put(map2_punc, ";");

    int total = 0;
    total += map2_res->num_toks;
    total += map2_lib->num_toks;
    total += map2_id->num_toks;
    total += map2_char->num_toks;
    total += map2_num->num_toks;
    total += map2_ptr->num_toks;
    total += map2_bkt->num_toks;
    total += map2_op->num_toks;
    total += map2_cmp->num_toks;
    total += map2_punc->num_toks;
    total += map2_fmt->num_toks;
    total += map2_prtd->num_toks;
    total += map2_cmmt->num_toks;
    total += map2_undef->num_toks;
    total += map2_skip->num_toks;
    printf("Total: %d tokens\n\n", total);

#define PRINT_MAP(map) \
    if (map->buckets) { \
        printf(#map ": %d\n", map->num_toks); \
        for (int i = 0; i < map->capacity; i++) { \
            HashEntry *ent = &map->buckets[i]; \
            if (ent->key) { \
                printf("%s", ent->key); \
                if (ent->val > 1) \
                    printf(" (x%d)", ent->val); \
                printf("\n"); \
            } \
        } \
        printf("\n"); \
    }

    PRINT_MAP(map2_res);
    PRINT_MAP(map2_lib);
    PRINT_MAP(map2_id);
    PRINT_MAP(map2_char);
    PRINT_MAP(map2_num);
    PRINT_MAP(map2_ptr);
    PRINT_MAP(map2_bkt);
    PRINT_MAP(map2_op);
    PRINT_MAP(map2_cmp);
    PRINT_MAP(map2_punc);
    PRINT_MAP(map2_fmt);
    PRINT_MAP(map2_prtd);
    PRINT_MAP(map2_cmmt);
    PRINT_MAP(map2_undef);
    PRINT_MAP(map2_skip);

    printf("OK\n\n");
}
