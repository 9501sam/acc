#include <regex.h>

#include "types.h"
#include "scanner.h"

/* REGEX FLAG */
#define ICASE REG_ICASE
#define NONE 0

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
    char       *pattern = "^";
    strcat(pattern, format);

    if (0 != (rc = regcomp(&preg, pattern, 0))) {
        printf("regcomp() failed, returning nonzero (%d)\n", rc);
        exit(EXIT_FAILURE);
    }

    if (0 != (rc = regexec(&preg, string, nmatch, pmatch, 0))) {
        fprintf(stderr, "Failed to match '%s' with '%s',returning %d.\n",
                string, pattern, rc);
        regfree(&preg);
        return 0;
    }
    printf("With the whole expression, "
            "a matched substring \"%.*s\" is found at position %d to %d.\n",
            pmatch[0].rm_eo - pmatch[0].rm_so, &string[pmatch[0].rm_so],
            pmatch[0].rm_so, pmatch[0].rm_eo - 1);
    regfree(&preg);
    return pmatch[0].rm_eo - pmatch[0].rm_so;
}

void skipspace(char *p)
{
    
}

token_t* scanning(char *path)
{
    char *input = extract_file(path);
    char *p = input;

    if (input == NULL) {
        fprintf(stderr, "[ERROR] input is NULL.\n");
        return NULL;
    }

    // read token
    while (*p) {
        if (isalpha(*p)){
            // Reserved Word
            if (match(p, "include", ICASE)){
            }
            if (match(p, "main", ICASE)){
            }
            if (match(p, "char", ICASE)){
            }
            if (match(p, "int", ICASE)){
            }
            if (match(p, "float", ICASE)){
            }
            if (match(p, "if", ICASE)){
            }
            if (match(p, "else", ICASE)){
            }
            if (match(p, "elseif", ICASE)){
            }
            if (match(p, "for", ICASE)){
            }
            if (match(p, "while", ICASE)){
            }
            if (match(p, "do", ICASE)){
            }
            if (match(p, "return", ICASE)){
            }
            if (match(p, "switch", ICASE)){
            }
            if (match(p, "case", ICASE)){
            }
            if (match(p, "printf", ICASE)){
            }
            if (match(p, "scanf", ICASE)){
            }
            // Identifier
            if (match(p, "[[:alpha:]][[:alnum:]_]*", NONE)) {
            }
        }
        // Library Name
        else if (match(p, "<[[:alpha:]]+\\.h>", NONE)) {
        }
        // Character
        else if (match(p, "'.'", NONE)) {
        }
        // Number '123' '123.123' '.123'
        else if (match(p, "([[:digit:]]+[.]?[[:digit:]]*)|([[:digit:]]*[.]?[[:digit:]]+)", NONE)){
        }
        // Number include bracket, optional sign symbol
        else if (match(p, "(\\([+-]?[[:digit:]]+[.]?[[:digit:]]*[[:blank:]]*\\))|(\\([[:blank:]]*[+-]?[[:digit:]]*[.]?[[:digit:]]+[[:blank:]]*\\))", NONE)) {
        }
        // Number include sign '+'
        else if (match(p, "(\\+[[:digit:]]+[.]?[[:digit:]]*)|(\\+[[:digit:]]*[.]?[[:digit:]]+)", NONE)) {
        }
        // Number include sign '-'
        else if (match(p, "(-[[:digit:]]+[.]?[[:digit:]]*)|(-[[:digit:]]*[.]?[[:digit:]]+)", NONE)) {
        }
        // Pointer
        else if (match(p, "\\*[[:alpha:]][[:alnum:]_]*", NONE)) {
        }
        // Bracket '('
        else if (match(p, "\\(", NONE)) {
        }
        // Bracket ')'
        else if (match(p, "\\)", NONE)) {
        }
        // Bracket '['
        else if (match(p, "\\[", NONE)) {
        }
        // Bracket ']'
        else if (match(p, "\\]", NONE)) {
        }
        // Bracket '{'
        else if (match(p, "\\{", NONE)) {
        }
        // Bracket '}'
        else if (match(p, "\\}", NONE)) {
        }
        // Operator '++'
        else if (match(p, "\\+\\+", NONE)) {
        }
        // Operator '+'
        else if (match(p, "\\+", NONE)) {
        }
        // Operator '--'
        else if (match(p, "--", NONE)) {
        }
        // Operator '-'
        else if (match(p, "-", NONE)) {
        }
        // Operator '*'
        else if (match(p, "\\*", NONE)) {
        }
        // Comment '//...'
        else if (match(p, "//.*", NONE)) {
        }
        // Operator '/'
        else if (match(p, "/", NONE)) {
        }
        // Format Specifier '%d' '%f' '%c'
        else if (match(p, "%[dfc]?", NONE)) {
        }
        // Operator '%'
        else if (match(p, "%", NONE)) {
        }
        // Operator '^'
        else if (match(p, "\\^", NONE)) {
        }
        // Address
        else if (match(p, "&[[:alpha:]][[:alnum:]_]*", NONE)) {
        }
        // Operator '&'
        else if (match(p, "&", NONE)) {
        }
        // Operator '|'
        else if (match(p, "\\|", NONE)) {
        }
        // Comparator '=='
        else if (match(p, "==", NONE)) {
        }
        // Operator '='
        else if (match(p, "=", NONE)) {
        }
        // Comparator '<='
        else if (match(p, "<=", NONE)) {
        }
        // Comparator '<'
        else if (match(p, "<", NONE)) {
        }
        // Comparator '>='
        else if (match(p, ">=", NONE)) {
        }
        // Comparator '>'
        else if (match(p, ">", NONE)) {
        }
        // Comparator '!='
        else if (match(p, "!=", NONE)) {
        }
        // Punctuation ','
        else if (match(p, ",", NONE)) {
        }
        // Punctuation ';'
        else if (match(p, ";", NONE)) {
        }
        // Punctuation ':'
        else if (match(p, ":", NONE)) {
        }
        // Punctuation '#'
        else if (match(p, "#", NONE)) {
        }
        // Punctuation '"'
        else if (match(p, "\"", NONE)) {
        }
        // Punctuation '''
        else if (match(p, "'", NONE)) {
        }
        // Format Specifier '\.'
        else if (match(p, "\\\\.", NONE)) {
        }
        // Comment '/*...*/'
        else if (match(p, "/\*.*\*/", NONE)) {
        }
        // Undefined token
        else if (match(p, "[[:graph:]*]", NONE)) {
        }

        /* TODO: Flag check
         *	1. if FLAG_FOUND is set, check condition 2
         *	2. if FLAG_PRINTED is set, every token except Format specifier will be Printed token
         *	3. if FLAG_UNDEFINED is set, Skipped token will be added after  Undefined token
         */

        /* TODO: skip space
         *	if pointer p encounter '\n', flag needs to reset
         */
        skipspace(p);
    }

    return NULL;
}
