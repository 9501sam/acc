#include <regex.h>

#include "types.h"
#include "scanner.h"
#include "set.h"
#include "flag.h"

/* REGEX FLAG */
#define ICASE REG_ICASE
#define NONE 0

HashMap *maps;
Flags *flags;

int token_count = 0; // TODO: bad practice in set.c

HashMap *declared_set;
static bool is_declared(char *key, int keylen)
{
    return get_entry(declared_set, key, keylen) != NULL;
}

static void declare(char *key, int keylen)
{
    hashmap_put(declared_set, key, keylen);
}

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
    cflags = cflags | REG_EXTENDED;

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

static bool is_reserved_word(char *p, int len)
{
    static char *kw[] = {
        "include", "main", "char", "int", "float", "if",
        "else", "elseif", "for", "while", "do", "return",
        "switch", "case", "printf", "scanf",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if (!strncasecmp(p, kw[i], len) && (len == strlen(kw[i])))
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

static int read_number(char *p)
{
    int len;

    // .123
    len = match(p, "[.][[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (!match(p + len, "[.[:alpha:]]", REG_EXTENDED | ICASE)) ? len : 0;

    // 123.123 or -123.123
    len = match(p, "[-]?[[:digit:]][[:digit:]]*[.][[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (!match(p + len, "[.[:alpha:]]", REG_EXTENDED | ICASE)) ? len : 0;

    // 123 or -123
    len = match(p, "[-]?[[:digit:]][[:digit:]]*", REG_EXTENDED);
    if (len)
        return (!match(p + len, "[.[:alpha:]]", REG_EXTENDED | ICASE)) ? len : 0;

    // (-123.123)
    len = match(p, "\\([-][[:digit:]][[:digit:]]*[.][[:digit:]][[:digit:]]*\\)", REG_EXTENDED);
    if (len)
        return (!match(p + len, "[.[:alpha:]]", REG_EXTENDED | ICASE)) ? len : 0;

    // (-123)
    len = match(p, "\\([-][[:digit:]][[:digit:]]*\\)", REG_EXTENDED);
    if (len)
        return len;

    return 0;
}

static int handle_undefine_token_then_skip(char *p, int len)
{
    char *q = p;

	// UNDEFINED_TOKEN
    maps_put(maps, q, len, UNDEFINED_TOKEN);
    q += len;

	// skip whitespace
	len = match(q, "[[:blank:]]*", NONE);
	q += len;

    // SKIPPED_TOKEN after UNDEFINED_TOKEN
	len = match(q, "[^\n]*", NONE);
    if (len) {
        maps_put(maps, q, len, SKIPPED_TOKEN);
        q += len;
    } 
    return q - p;
}

/* handle flags */

/* skip */
// skip newline
// skip whitespace

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

token_t* scanning(char *path)
{
	/* initialization */
    char *input = extract_file(path);
    char *p = input;
    int len;
    flags = flag_init();
    token_count = 0;

    if (input == NULL) {
        fprintf(stderr, "[ERROR] input is NULL.\n");
        return NULL;
    }

    maps = calloc(NUM_TYPES, sizeof(HashMap));
    declared_set = calloc(1, sizeof(HashMap));


	/* start reading file from *p */
    while (*p) {
        /* handle flags */
		if (match(p, "(int|float|char)", ICASE)) {
			flag_on(flags, FLAG_DECLARE);
		}

        if (match(p, "#[[:space:]]*include", ICASE)) {
            flag_on(flags, FLAG_INCLUDE);
        }

		// TODO this code will cause any token to be FORMAT_SPECIFIER after any '"' when flag FLAG_PRINTED is on
        if (*p == '"') {
            flag_switch(flags, FLAG_PRINTED);
        }

        if ((*p == ';') || (*p == '\n')) {
            flag_reset(flags);
        }

        /* skip space */
		len = match(p, "[[:space:]]", NONE);
		if (len) {
			p++;
			continue;
		}

		// TODO not handle yet
        /* string */
        // PRINTED_TOKEN,
        // FORMAT_SPECIFIER,
        if (*p == '"') {
            maps_put(maps, p, strlen("\""), PUNCTUATION);
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
                    maps_put(maps, p, len, FORMAT_SPECIFIER);
                    p += len;
                    continue;
                }

                // PRINTED_TOKEN
                len = match(p, "[^[:space:]\"]*", NONE);
                if (len) {
                    maps_put(maps, p, len, PRINTED_TOKEN);
                    p += len;
                }
            }
            if (*p == '"') {
                maps_put(maps, p, strlen("\""), PUNCTUATION);
                p++;
            }
            continue;
        }

        /* combination */ 
        // CHARACTER
        len = match(p, "'[[:print:]]'", NONE);
        if (len) {
            maps_put(maps, p, len, CHARACTER);
            p += len;
            continue;
        }

        // COMMENT
        len = match(p, "(//[^\n]*|/[*]([^*]|[*][^/])*[*]/)", NONE);
        if (len) {
            maps_put(maps, p, len, COMMENT);
            p += len;
            continue;
        }

		// TODO not handle yet
        // POINTER
        if (*p == '*' && isalpha(p[1])) {
            len = read_pointer(p);
            if (len) {
                if (flag_isset(flags, FLAG_DECLARE)) { // int *abc;
                    declare(p + 1, len - 1); // abc
                    maps_put(maps, p, len, POINTER);
                    p += len;

                } else if (is_declared(p + 1, len - 1)) { // *abc = 10;
                    maps_put(maps, p, len, POINTER);
                    p += len;

                } else {
                    len = handle_undefine_token_then_skip(p, len);
                    p += len;
                }
                continue;
            }
        }

		// TODO not handle yet
        // ADDRESS,
        if (*p == '&' && isalpha(p[1])) {
            len = read_address(p);
            if (len) {
                if (is_declared(p + 1, len - 1)) { // return &abc;
                    maps_put(maps, p, len, ADDRESS);
                    p += len;

                } else {
                    len = handle_undefine_token_then_skip(p, len);
                    p += len;
                }
                continue;
            }
        }

        // LIBRARY_NAME,
        len = match(p, "<[[:alpha:]][[:alnum:]_]*[.]h>", ICASE);
        if (len) {
            if (flag_isset(flags, FLAG_INCLUDE)) {
                maps_put(maps, p, len, LIBRARY_NAME);
                p += len;
                continue;
            }
        }

		// TODO not handle yet
        /* number */ 
        // NUMBER,
        len = read_number(p);
        if (len) {
            maps_put(maps, p, len, NUMBER);
            p += len;
            continue;
        }

        /* other */ 
        // BRACKET
        len = match(p, "[]()[{}]", NONE);
        if (len) {
            maps_put(maps, p, len, BRACKET);
            p++;
            continue;
        }

        // COMPARATOR,
        len = match(p, "(<|>|>=|<=|==|!=)", NONE);
        if (len) {
            maps_put(maps, p, len, COMPARATOR);
            p += len;
            continue;
        }

        // OPERATOR,
		len = match(p, "([+][+]|--|[+]|-|[*]|[/]|\\^|%|&|[|]|=)", NONE);
        if (len) {
            maps_put(maps, p, len, OPERATOR);
            p += len;
            continue;
        }

        // PUNCTUATION,
        len = match(p, "[,;:#\"']", NONE);
        if (len) {
            maps_put(maps, p, len, PUNCTUATION);
            p += len;
            continue;
        }

        /* alphabet */ 
        // RESERVED_WORD,
		len = match(p, "(include|main|char|int|float|if|else|elseif|for|while|do|return|switch|case|printf|scanf)", ICASE);
		if (len) {
			maps_put(maps, p, len, RESERVED_WORD);
			p += len;
			continue;
		}

        // IDENTIFIER,
        len = match(p, "[[:alpha:]][[:alnum:]_]*", NONE);
        if (len) {
            if (flag_isset(flags, FLAG_DECLARE)) { // int abc;
                declare(p, len);
                maps_put(maps, p, len, IDENTIFIER);
                p += len;
            } else if (is_declared(p, len)) { // abc = 10;
                maps_put(maps, p, len, IDENTIFIER);
                p += len;
            } else {
                len = handle_undefine_token_then_skip(p, len);
                p += len;
            }
            continue;
        }

        /* ?! */ 
        // UNDEFINED_TOKEN,
        len = match(p, "[[:alnum:]_\\.]*", NONE);
        if (len) {
            len = handle_undefine_token_then_skip(p, len);
            p += len;
        } else { // should not reach here
            fprintf(stderr, "scanning error");
            exit(EXIT_FAILURE);
        }
    }

    print_maps(maps);
    return NULL;
}

void scanner_test(void)
{
    // scanning("test/test1.c");
    scanning("test/test2.c");
    // scanning("test/test3.c");
    // scanning("test/test3.c");
    // scanning("test/test_num.c");
    // scanning("test/test_string.c");

    // match
    int len;

    len = match("asdf", "asdf", NONE);
    assert(len == 4);

    len = match("asdf", "0asdf", NONE);
    assert(len == 0);

    len = match("asdf", "aSDf", ICASE);
    assert(len == 4);

	len = match("iNT", "(int|char|float)", ICASE);
	assert(len == 3);
}
