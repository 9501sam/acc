#include <regex.h>

#include "types.h"
#include "scanner.h"
#include "set.h"
#include "flag.h"

/* REGEX FLAG */
#define ICASE REG_ICASE
#define NONE 0

Flags *flags;

static char* extract_file(char *path)
{
    FILE *pfile = fopen(path, "r");

    if (pfile == NULL){
        fprintf(stderr, "[ERROR] File %s doesn't exist.\n", path);
        return NULL;
    }

    int length;
    char *buffer;

    fseek(pfile, 0, SEEK_END);
    length = ftell(pfile);
    rewind(pfile);
    buffer = (char*)malloc((length + 5) * sizeof(char));

    if (!buffer){
        fprintf(stderr, "[ERROR] Failed to allocate memory to buffer.\n");
        return NULL;
    }

    fread(buffer, 1, length, pfile);
    fclose(pfile);
    buffer[length - 1] = '\n';
    buffer[length] = '\n';
    buffer[length + 1] = '\0';
    buffer[length + 2] = '\0';
    buffer[length + 3] = '\0';
    buffer[length + 4] = '\0';
    return buffer;
}

static bool startswith(char *p, char *q)
{
    return strncmp(p, q, strlen(q)) == 0;
}

static int match(char *p, char *format, int cflags)
{
    regex_t    preg;
    char       *string = p;
    int        rc;
    size_t     nmatch = 1;
    regmatch_t pmatch[1];
    char       pattern[strlen(format) + 2];
    pattern[0] = '^';
    pattern[1] = '\0';
    strncat(pattern, format, strlen(format));

    if (0 != (rc = regcomp(&preg, pattern, cflags))) {
        printf("regcomp() failed, returning nonzero (%d)\n", rc);
        exit(EXIT_FAILURE);
    }

    if (0 != (rc = regexec(&preg, string, nmatch, pmatch, 0))) {
        regfree(&preg);
        return 0;
    }
    regfree(&preg);
    return pmatch[0].rm_eo - pmatch[0].rm_so;
}

static int read_character(char *p)
{
    if (p[0] == '\'' && p[2] == '\'')
        return 3;
    return 0;
}

static int read_comment(char *p)
{
    char *q = p;

    // line comments.
    if (startswith(p, "//")) {
        q += 2;
        while (*q != '\n')
            q++;
        return q - p;
    }

    // block comments.
    if (startswith(p, "/*")) {
        char *q = strstr(p + 2, "*/");
        if (!q) { // TODO: might be unclosed
            return 0;
        }
        q += 2;
        return q - p;
    }

    return 0;
}

static bool is_reserved_word(char *p, int len)
{
    static char *kw[] = {
        "include", "main", "char", "int", "float", "if",
        "else", "elseif", "for", "while", "do", "return",
        "switch", "case", "printf", "scanf",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (!strncasecmp(p, kw[i], len))
            return true;
    return false;
}

// IDENTIFIER or RESERVED_WORD
static int read_word(char *p)
{
    int len = match(p, "[[:alpha:]][[:alnum:]_]*", ICASE);
    if (*(p + len) != '.')
        return len;
    return 0;
}

static int read_pointer(char *p)
{
    char *q = p;
    int id_len;

    if (*q != '*')
        return 0;
    q++;
    id_len = read_word(q);

    if (id_len)
        return id_len + 1;
    return 0;
}

static int read_address(char *p)
{
    char *q = p;
    int id_len;

    if (*q != '&')
        return 0;
    q++;
    id_len = read_word(q);

    if (id_len)
        return id_len + 1;
    return 0;
}

// <xxx.h>
static int read_library(char *p)
{
    char *q = p;

    if (*q != '<')
        return 0;
    q++;
    while (!startswith(q, ".h>") && isalpha(*q))
        q++;
    if (startswith(q, ".h>")) {
        q += strlen(".h>");
        return q - p;
    }
    return 0; // undefine token
}

static int read_number(char *p)
{
    int len;

    // .123
    len = match(p, "[.][[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (p[len] != '.') ? len : 0;

    // 123.123 or -123.123
    len = match(p, "[-]?[[:digit:]][[:digit:]]*[.][[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (p[len] != '.') ? len : 0;

    // 123 or -123
    len = match(p, "[-]?[[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (p[len] != '.') ? len : 0;

    // (-123.123)
    len = match(p, "\\([-][[:digit:]][[:digit:]]*[.][[:digit:]][[:digit:]]*\\)", REG_EXTENDED);
    if (len)
        return len;

    // (-123)
    len = match(p, "\\([-][[:digit:]][[:digit:]]*\\)", REG_EXTENDED);
    if (len)
        return len;

    return 0;
}

static int read_bracket(char *start)
{
    if ((*start == '(') | (*start == ')') |
            (*start == '[') | (*start == ']') |
            (*start == '{') | (*start == '}'))
        return 1;
    return 0;
}

static int read_operator(char *p) {
    static char *kw[] = {
        "++", "--",
        "+", "-", "*", "/", "%", "^", "&", "|", "=",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (startswith(p, kw[i]))
            return strlen(kw[i]);
    return 0;
}

static int read_comparator(char *p)
{
    static char *kw[] = {
        "==", "<=", ">=", "!=",
        "<", ">",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (startswith(p, kw[i]))
            return strlen(kw[i]);
    return 0;
}

static int read_punctuation(char *p)
{
    static char *kw[] = {
        ",", ";", ":","#", "\"", "'"
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (startswith(p, kw[i]))
            return strlen(kw[i]);
    return 0;
}

static int read_undefine_token(char *p)
{
    return match(p, "[[:alnum:]_\\.]*", ICASE | REG_EXTENDED);
}

static int read_skipped_token(char *p)
{
    char *q = p;

    while (*q != '\n')
        q++;
    return q - p;
}

token_t* scanning(char *path)
{
    char *input = extract_file(path);
    char *p = input;
    int len;
    flags = flag_init();

    if (input == NULL) {
        fprintf(stderr, "[ERROR] input is NULL.\n");
        return NULL;
    }

    HashMap *map = calloc(NUM_TYPES, sizeof(HashMap));

    while (*p) {
        // newline
        // whitespace

        /* string */ 
        // PRINTED_TOKEN,
        // FORMAT_SPECIFIER,

        /* combination */ 
        // CHARACTER,
        // COMMENT, // UNDEF: unclosed comment
        // POINTER, // UNDEF: not found
        // ADDRESS, // UNDEF: not found
        // LIBRARY_NAME, // UNDEF

        /* number */ 
        // NUMBER, // UNDEF

        /* other */ 
        // BRACKET,
        // OPERATOR,
        // COMPARATOR,
        // PUNCTUATION,

        /* alphabet */ 
        // RESERVED_WORD, // UNDEF
        // IDENTIFIER, // UNDEF: not found

        /* ?! */ 
        // UNDEFINED_TOKEN,
        // SKIPPED_TOKEN,

        // Skip newline.
        if (*p == '\n') {
            p++;
            continue;
        }

        // Skip whitespace characters.
        if (*p == ' ') {
            p++;
            continue;
        }

        /* string */ 
        // PRINTED_TOKEN,
        // FORMAT_SPECIFIER,
        if (*p == '"') {
            hashmap_put(&map[PUNCTUATION], p, strlen("\""));
            p++;
            while (*p != '"' && *p != '\n') {
                if (isspace(*p)) {
                    p++;
                    continue;
                }

                // FORMAT_SPECIFIER,
                if ((*p == '\\') || 
                        match(p, "%d", NONE) ||
                        match(p, "%f", NONE) ||
                        match(p, "%c", NONE)) {
                    len = 2;
                    hashmap_put(&map[FORMAT_SPECIFIER], p, len);
                    p += len;
                    continue;
                }

                // PRINTED_TOKEN
                len = match(p, "[^[:space:]\"]*", NONE);
                if (len) {
                    hashmap_put(&map[PRINTED_TOKEN], p, len);
                    p += len;
                }

            }
            if (*p == '"') {
                hashmap_put(&map[PUNCTUATION], p, strlen("\""));
                p++;
            }
            continue;
        }

        /* combination */ 
        // CHARACTER,
        len = read_character(p);
        if (len) {
            hashmap_put(&map[CHARACTER], p, len);
            p += len;
            continue;
        }

        // COMMENT,
        len = read_comment(p);
        if (len) {
            hashmap_put(&map[COMMENT], p, len);
            p += len;
            continue;
        }

        // POINTER,
        if (*p == '*' && isalpha(p[1])) {
            len = read_pointer(p);
            if (len) {
                hashmap_put(&map[POINTER], p, len);
                p += len;
                continue;
            }
        }

        // ADDRESS,
        if (*p == '&' && isalpha(p[1])) {
            len = read_address(p);
            if (len) {
                hashmap_put(&map[ADDRESS], p, len);
                p += len;
                continue;
            }
        }

        // LIBRARY_NAME,
        len = read_library(p);
        if (len) {
            hashmap_put(&map[LIBRARY_NAME], p, len);
            p += len;
            continue;
        }

        /* number */ 
        // NUMBER,
        len = read_number(p);
        if (len) {
            hashmap_put(&map[NUMBER], p, len);
            p += len;
            continue;
        }

        /* other */ 
        // BRACKET
        len = read_bracket(p);
        if (len) {
            hashmap_put(&map[BRACKET], p, len);
            p++;
            continue;
        }

        // COMPARATOR,
        len = read_comparator(p);
        if (len) {
            hashmap_put(&map[COMPARATOR], p, len);
            p += len;
            continue;
        }

        // OPERATOR,
        len = read_operator(p);
        if (len) {
            hashmap_put(&map[OPERATOR], p, len);
            p += len;
            continue;
        }

        // PUNCTUATION,
        len = read_punctuation(p);
        if (len) {
            hashmap_put(&map[PUNCTUATION], p, len);
            p += len;
            continue;
        }

        /* alphabet */ 
        // RESERVED_WORD,
        // IDENTIFIER,
        len = read_word(p);
        if (len) {
            if (is_reserved_word(p, len))
                hashmap_put(&map[RESERVED_WORD], p, len);
            else
                hashmap_put(&map[IDENTIFIER], p, len);
            p += len;
            continue;
        }

        /* ?! */ 
        // UNDEFINED_TOKEN,
        len = read_undefine_token(p);
        if (len) {
            hashmap_put(&map[UNDEFINED_TOKEN], p, len);
            p += len;

            // SKIPPED_TOKEN after UNDEFINED_TOKEN
            while (*p == ' ')
                p++;
            len = read_skipped_token(p);
            if (len) {
                printf("len = %d\n", len);
                hashmap_put(&map[SKIPPED_TOKEN], p, len);
                p += len;
                continue;

            } 
        } else { // shoud not reach here
            fprintf(stderr, "scanning error");
            exit(EXIT_FAILURE);
        }
    }

    print_maps(map);
    return NULL;
}

void scanner_test(void)
{
    scanning("test/test1.c");
    scanning("test/test2.c");
    scanning("test/test3.c");
    scanning("test/test_num.c");
    scanning("test/test_string.c");

    // match
    int len;

    len = match("asdf", "asdf", NONE);
    assert(len == 4);

    len = match("asdf", "0asdf", NONE);
    assert(len == 0);

    len = match("asdf", "aSDf", ICASE);
    assert(len == 4);
}
