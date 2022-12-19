#include "set.h"
#include "types.h"

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
    for (int i = 0; i < map->num_ents; i++)
        if (map->buckets[i].key)
            nkeys++;

    int cap = map->num_ents;
    while ((nkeys * 100) / cap >= LOW_WATERMARK)
        cap = cap * 2;
    assert(cap > 0);

    // Create a new hashmap and copy all key-values.
    HashMap map2 = {};
    map2.buckets = calloc(cap, sizeof(HashEntry));
    map2.num_ents = cap;
    map2.num_toks = map->num_toks;

    for (int i = 0; i < map->num_ents; i++) {
        HashEntry *ent = &map->buckets[i];
        if (ent->key)
            hashmap_put2(&map2, ent->key, ent->keylen, ent->val);
    }

    assert(map2.num_used_ents == nkeys);
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

    for (int i = 0; i < map->num_ents; i++) {
        HashEntry *ent = &map->buckets[(hash + i) % map->num_ents];
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
        map->num_ents = INIT_SIZE;
        map->num_toks = 0;
    } else if ((map->num_used_ents * 100) / map->num_ents >= HIGH_WATERMARK) {
        rehash(map);
    }

    uint64_t hash = fnv_hash(key, keylen);

    for (int i = 0; i < map->num_ents; i++) {
        HashEntry *ent = &map->buckets[(hash + i) % map->num_ents];

        if (match(ent, key, keylen))
            return ent;

        if (ent->key == NULL) {
            ent->key = key;
            ent->keylen = keylen;
            ent->val = 0;
            map->num_used_ents++;
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

void hashmap_put(HashMap *map, char *key, int keylen)
{
    HashEntry *ent = get_or_insert_entry(map, key, keylen);
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

const char *type_names[NUM_TYPES] = {
    "Reserved word",
    "Library name",
    "Identifier",
    "Character",
    "Number",
    "Pointer",
    "Bracket",
    "Operator",
    "Comparator",
    "Address",
    "Punctuation",
    "Format specifier",
    "Printed token",
    "Comment",
    "Undefined token",
    "Skipped token",
};

static void print_maps(HashMap maps[])
{
    int total = 0;
    for (int i = 0; i < NUM_TYPES; i++)
        if (maps[i].buckets)
            total += maps[i].num_toks;
    printf("Total: %d tokens\n\n", total);

    for (int i = 0; i < NUM_TYPES; i++) {
        HashMap *map = &maps[i];
        if (!map->buckets)
            continue;
        printf("%s: %d\n", type_names[i], map->num_toks);
        for (int i = 0; i < map->num_ents; i++) {
            HashEntry *ent = &map->buckets[i];
            if (!ent->key)
                continue;

            char key[ent->keylen + 1];
            strncpy(key, ent->key, ent->keylen);
            key[ent->keylen] = '\0';
            printf("%s", key);
            if (ent->val > 1)
                printf(" (x%d)", ent->val);
            printf("\n");
        }
        printf("\n");
    }
}

void hashmap_test(void)
{
    printf("======test hashmap======\n");

    HashMap *map = calloc(1, sizeof(HashMap));

    char *key;
    for (int i = 0; i < 5000; i++) {
        key = format("key %d", i);
        hashmap_put(map, key, strlen(key));
        assert(map->num_toks == (i + 1));
    }
    for (int i = 0; i < 5000; i++) {
        key = format("key %d", i);
        assert(hashmap_get(map, key) == 1);
    }
    for (int i = 4000; i < 5000; i++) {
        key = format("key %d", i);
        hashmap_put(map, key, strlen(key));
    }
    for (int i = 0; i < 4000; i++)
        assert(hashmap_get(map, format("key %d", i)) == 1);
    for (int i = 4000; i < 5000; i++)
        assert(hashmap_get(map, format("key %d", i)) == 2);
    assert(map->num_used_ents == 5000);
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
    HashMap *map2 = calloc(NUM_TYPES, sizeof(HashMap));
    hashmap_put(&map2[COMPARATOR], "<", strlen("<"));
    hashmap_put(&map2[UNDEFINED_TOKEN], "stdio.h", strlen("stdio.h"));
    hashmap_put(&map2[SKIPPED_TOKEN], ">", strlen(">"));
    hashmap_put(&map2[RESERVED_WORD], "int", strlen("int"));
    hashmap_put(&map2[IDENTIFIER], "ia", strlen("ia"));
    hashmap_put(&map2[PUNCTUATION], ",", strlen(","));
    hashmap_put(&map2[POINTER], "*ie", strlen("*ie"));
    hashmap_put(&map2[PUNCTUATION], ",", strlen(","));
    hashmap_put(&map2[IDENTIFIER], "forif", strlen("forif"));
    hashmap_put(&map2[PUNCTUATION], ";", strlen(";"));
    hashmap_put(&map2[RESERVED_WORD], "char", strlen("char"));
    hashmap_put(&map2[IDENTIFIER], "ca", strlen("ca"));
    hashmap_put(&map2[PUNCTUATION], ";", strlen(";"));
    hashmap_put(&map2[IDENTIFIER], "ia", strlen("ia"));
    hashmap_put(&map2[OPERATOR], "=", strlen("="));
    hashmap_put(&map2[BRACKET], "(", strlen("("));
    hashmap_put(&map2[POINTER], "*ie", strlen("*ie"));
    hashmap_put(&map2[BRACKET], ")", strlen(")"));
    hashmap_put(&map2[OPERATOR], "+", strlen("+"));
    hashmap_put(&map2[UNDEFINED_TOKEN], "ib", strlen("ib"));
    hashmap_put(&map2[SKIPPED_TOKEN], "; ia = ia *2;", strlen("; ia = ia *2;"));
    hashmap_put(&map2[IDENTIFIER], "ca", strlen("ca"));
    hashmap_put(&map2[OPERATOR], "=", strlen("="));
    hashmap_put(&map2[CHARACTER], "'A'", strlen("'A'"));
    hashmap_put(&map2[PUNCTUATION], ";", strlen(";"));
    hashmap_put(&map2[UNDEFINED_TOKEN], "whilefor", strlen("whilefor"));
    hashmap_put(&map2[SKIPPED_TOKEN], "(ia<3) forif = forif + 1;", strlen( "(a<3) forif = forif + 1;"));
    hashmap_put(&map2[RESERVED_WORD], "sCAnf", strlen("sCAnf"));
    hashmap_put(&map2[BRACKET], "(", strlen( "("));
    hashmap_put(&map2[PUNCTUATION], "\"", strlen("\""));
    hashmap_put(&map2[FORMAT_SPECIFIER], "%c", strlen("%c"));
    hashmap_put(&map2[PUNCTUATION], "\"", strlen("\""));
    hashmap_put(&map2[PUNCTUATION], ",", strlen(","));
    hashmap_put(&map2[UNDEFINED_TOKEN], "&cb", strlen("&cb"));
    hashmap_put(&map2[SKIPPED_TOKEN], ");", strlen(");"));
    hashmap_put(&map2[IDENTIFIER], "ia", strlen("ia"));
    hashmap_put(&map2[OPERATOR], "=", strlen("="));
    hashmap_put(&map2[OPERATOR], "*", strlen("*"));
    hashmap_put(&map2[IDENTIFIER], "forif", strlen("forif"));
    hashmap_put(&map2[OPERATOR], "+", strlen("+"));
    hashmap_put(&map2[NUMBER], "(-2)", strlen("(-2)"));
    hashmap_put(&map2[PUNCTUATION], ";", strlen(";"));
    hashmap_put(&map2[IDENTIFIER], "ia", strlen("ia"));
    hashmap_put(&map2[OPERATOR], "=", strlen("="));
    hashmap_put(&map2[NUMBER], "5", strlen("5"));
    hashmap_put(&map2[POINTER], "*ie", strlen("*ie"));
    hashmap_put(&map2[PUNCTUATION], ";", strlen(";"));
    print_maps(map2);

    // int a, bb, ccc, dddd;
    printf("========\n");
    printf("int a, bb, ccc, dddd;\n\n");

    typedef struct Tok Tok;
    struct Tok {
        Tok *next;
        char *start_locaiton;
        int len;
    };

    Tok *new_token(char *start, int len)
    {
        Tok *t = calloc(1, sizeof(Tok));
        t->start_locaiton = start;
        t->len = len;
        return t;
    }

    // return length of reserved word
    int read_reserved(const char *start)
    {
        if (!strncmp(start, "int", 3)) {
            return 3;
        }
        return 0;
    }

    // return length of reserved identifier
    int read_id(const char *start)
    {
        char *ptr = start;
        while (*ptr && 
                *ptr != ' ' &&
                *ptr != ',' &&
                *ptr != ';')
            ptr++;
        return ptr - start;
    }

    // return length of punctuator
    int read_punc(const char *start)
    {
        if ((*start == ',') || (*start == ';'))
            return 1;
        return 0;
    }

    Tok head = {};
    Tok *cur = &head;
    char *src = "int a, bb, ccc, dddd;"; // read from file
    char *p = src;
    HashMap *map3 = calloc(NUM_TYPES, sizeof(HashMap));

    while (*p) {
        // reserved word
        int res_len = read_reserved(p);
        if (res_len) {
            hashmap_put(&map3[RESERVED_WORD], p, res_len);
            cur = cur->next = new_token(p, res_len);
            p += res_len;
            continue;
        }

        // identifier
        int id_len = read_id(p);
        if (id_len) {
            hashmap_put(&map3[IDENTIFIER], p, id_len);
            cur = cur->next = new_token(p, id_len);
            p += id_len;
            continue;
        }

        // punctuators
        int punc_len = read_punc(p);
        if (punc_len) {
            hashmap_put(&map3[PUNCTUATION], p, punc_len);
            cur = cur->next = new_token(p, punc_len);
            p += punc_len;
            continue;
        }

        p++; // space
    }
    print_maps(map3);

    printf("OK\n\n");
}
