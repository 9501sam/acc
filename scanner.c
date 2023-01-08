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
	buffer = (char*)malloc(length * sizeof(char));

    if (!buffer){
        fprintf(stderr, "[ERROR] Failed to allocate memory to buffer.\n");
        return NULL;
    }

    fread(buffer, 1, length, pfile);
    fclose(pfile);
    return buffer;
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

static bool find_printf_or_scanf(char *p, char *start)
{
	char *q = p;

	while (q >= start && *q != ';' && *q != '\n') {
		if (match(q, "(printf|scanf)", ICASE))
			return true;
		q--;
	}
	return false;
}
	
static int handle_printf_and_scanf(char *p)
{
	char *q = p;
	int len;
	maps_put(maps, q, strlen("\""), PUNCTUATION);
	q++;
	while (1) {
		/* skip blank */
		len = match(q, "[[:blank:]]*", NONE);
		q += len;

		if (*q == '"' || *q == '\n')
			break;

		// FORMAT_SPECIFIER,
		len = match(q, "(%[dfc]|[\\][[:graph:]])", NONE);
		if (len) {
			maps_put(maps, q, len, FORMAT_SPECIFIER);
			q += len;
			continue;
		}

		// PRINTED_TOKEN
		
		// len = match(q, "[[:graph:]]+", NONE);
		len = 0;
		while (1) {
			if (match(q + len, "%[dfc]", NONE))
				break;
			if (match(q + len, "\\[[:graph:]]", NONE))
				break;
			if (match(q + len, "[[:space:]]", NONE))
				break;
			len++;
		}
		if (len) {
			maps_put(maps, q, len, PRINTED_TOKEN);
			q += len;
		}
	}
	if (*q == '"') {
		maps_put(maps, q, strlen("\""), PUNCTUATION);
		q++;
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
    char *start = extract_file(path);
    char *p = start;
    int len;
    flags = flag_init();
    token_count = 0;

    if (start == NULL) {
        fprintf(stderr, "[ERROR] start is NULL.\n");
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

        if (*p == '"' && find_printf_or_scanf(p, start)) {
            flag_on(flags, FLAG_PRINTED);
        }

        if ((*p == ';') || (*p == '\n')) {
            flag_reset(flags);
        }

        /* skip space */
		if (match(p, "[[:space:]]", NONE)) {
			p++;
			continue;
		}

        /* string */
        // PRINTED_TOKEN,
        // FORMAT_SPECIFIER,
        if (*p == '"' && flag_isset(flags, FLAG_PRINTED)) {
			len = handle_printf_and_scanf(p);
			p += len;
			flag_off(flags, FLAG_PRINTED);
			continue;
		}

        /* combination */ 
        // CHARACTER
		// QUESTION: character can what?
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

        // POINTER
		len = match(p, "[*][[:alpha:]][[:alnum:]_]*", NONE);
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

        // ADDRESS,
		len = match(p, "[&][[:alpha:]][[:alnum:]_]*", NONE);
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

        // LIBRARY_NAME
        len = match(p, "<[[:alpha:]][[:alnum:]_]*[.]h>", ICASE);
        if (len) {
            if (flag_isset(flags, FLAG_INCLUDE)) {
                maps_put(maps, p, len, LIBRARY_NAME);
                p += len;
                continue;
            }
        }

        /* number */ 
        // NUMBER
		len = match(p, "([-]?[[:blank:]]*[[:digit:]]+[.]?[[:digit:]]*|[-]?[[:blank:]]*[[:digit:]]*[.]?[[:digit:]]+|[(][[:blank:]]*-[[:blank:]]*[[:digit:]]+[.]?[[:digit:]]*[[:blank:]]*[)]|[(][[:blank:]]*-[[:blank:]]*[[:digit:]]*[.]?[[:digit:]]+[[:blank:]]*[)])", NONE);
		// need len > 0 and number can't follow with any alpha or '_'
        if (len && match(p + len, "[[:alpha:]_]+", NONE) == 0) {
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
    scanning("test/test1.c");
    scanning("test/test2.c");
    scanning("test/test3.c");
    scanning("test/test4.c");
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

	len = match("iNT", "(int|char|float)", ICASE);
	assert(len == 3);
}
