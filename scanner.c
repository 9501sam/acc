#include "types.h"
#include "scanner.h"

/* REGEX FLAG */
#define ICASE REG_ICASE
#define NONE 0

/* PRIVATE FUNCTION */
char* extract_file(char *path);
bool startswith(char *p, char *q);
// TODO
bool match(char *p, char *format, int cflags);
// TODO
void skipspace(char *p);

/****************************************
***        PUBLIC FUNCTION
****************************************/

token_t* scanning(char *path)
{
	char *input = extract_file(path);
	char *cur = input;
	
	if (input == NULL){
		fprintf("[ERROR] input is NULL.\n");
		return NULL;
	}

	// read token
	while (*cur){
		if (isalpha(*cur)){
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
			if (match(p, "[[:alpha:]][[:alnum:]_]*", NONE)){
			}
		}
		// Library Name
		else if (match(p, "<[[:alpha:]]+\\.h>", NONE)){
		}
		// Character
		else if (match(p, "'.'", NONE)){
		}
		// Number '123' '123.123' '.123'
		else if (match(p, "([[:digit:]]+[.]?[[:digit:]]*)|([[:digit:]]*[.]?[[:digit:]]+)", NONE)){
		}
		// Number include bracket, optional sign symbol
		else if (match(p, "(\\([+-]?[[:digit:]]+[.]?[[:digit:]]*[[:blank:]]*\\))|(\\([[:blank:]]*[+-]?[[:digit:]]*[.]?[[:digit:]]+[[:blank:]]*\\))", NONE)){
		}
		// Number include sign '+'
		else if (match(p, "(\\+[[:digit:]]+[.]?[[:digit:]]*)|(\\+[[:digit:]]*[.]?[[:digit:]]+)", NONE)){
		}
		// Number include sign '-'
		else if (match(p, "(-[[:digit:]]+[.]?[[:digit:]]*)|(-[[:digit:]]*[.]?[[:digit:]]+)", NONE)){
		}
		// Pointer
		else if (match(p, "\\*[[:alpha:]][[:alnum:]_]*", NONE)){
		}
		// Bracket '('
		else if (match(p, "\\(", NONE)){
		}
		// Bracket ')'
		else if (match(p, "\\)", NONE)){
		}
		// Bracket '['
		else if (match(p, "\\[", NONE)){
		}
		// Bracket ']'
		else if (match(p, "\\]", NONE)){
		}
		// Bracket '{'
		else if (match(p, "\\{", NONE)){
		}
		// Bracket '}'
		else if (match(p, "\\}", NONE)){
		}
		// Operator '++'
		else if (match(p, "\\+\\+", NONE)){
		}
		// Operator '+'
		else if (match(p, "\\+", NONE)){
		}
		// Operator '--'
		else if (match(p, "--", NONE)){
		}
		// Operator '-'
		else if (match(p, "-", NONE)){
		}
		// Operator '*'
		else if (match(p, "\\*", NONE)){
		}
		// Comment '//...'
		else if (match(p, "//.*", NONE)){
		}
		// Operator '/'
		else if (match(p, "/", NONE)){
		}
		// Format Specifier '%d' '%f' '%c'
		else if (match(p, "%[dfc]?", NONE)){
		}
		// Operator '%'
		else if (match(p, "%", NONE)){
		}
		// Operator '^'
		else if (match(p, "\\^", NONE)){
		}
		// Address
		else if (match(p, "&[[:alpha:]][[:alnum:]_]*", NONE)){
		}
		// Operator '&'
		else if (match(p, "&", NONE)){
		}
		// Operator '|'
		else if (match(p, "\\|", NONE)){
		}
		// Comparator '=='
		else if (match(p, "==", NONE)){
		}
		// Operator '='
		else if (match(p, "=", NONE)){
		}
		// Comparator '<='
		else if (match(p, "<=", NONE)){
		}
		// Comparator '<'
		else if (match(p, "<", NONE)){
		}
		// Comparator '>='
		else if (match(p, ">=", NONE)){
		}
		// Comparator '>'
		else if (match(p, ">", NONE)){
		}
		// Comparator '!='
		else if (match(p, "!=", NONE)){
		}
		// Punctuation ','
		else if (match(p, ",", NONE)){
		}
		// Punctuation ';'
		else if (match(p, ";", NONE)){
		}
		// Punctuation ':'
		else if (match(p, ":", NONE)){
		}
		// Punctuation '#'
		else if (match(p, "#", NONE)){
		}
		// Punctuation '"'
		else if (match(p, "\"", NONE)){
		}
		// Punctuation '''
		else if (match(p, "'", NONE)){
		}
		// Format Specifier '\.'
		else if (match(p, "\\\\.", NONE)){
		}
		// Comment '/*...*/'
		else if (match(p, "/\*.*\*/", NONE)){
		}
		// Undefined token
		else if (match(p, "[[:graph:]*]", NONE)){
		}
		
		/* TODO: Flag check
		 *	1. if FLAG_FOUND is set, check condition 2
		 *	2. if FLAG_PRINTED is set, every token except Format specifier will be Printed token
		 *	3. if FLAG_UNDEFINED is set, Skipped token will be added after  Undefined token
		 */

		/* TODO: skip space
		 *	if pointer p encounter '\n', flag needs to reset
		 */
		skipspace();
	}
			
    return NULL;
}


/****************************************
***        PRIVATE FUNCTION
****************************************/

char* extract_file(char *path){
	FILE *pfile = fopen(path, "r");
	
	if (pfile == NULL){
		fprintf("[ERROR] File %s doesn't exist.\n", path);
		return NULL;
	}

	int length;
	char *buffer;

	fseek(pfile, 0, SEEK_END);
	length = ftell(pfile);
	rewind(pfile);
	buffer = (char*)malloc(length * sizeof(char));

	if (!buffer){
		fprintf("[ERROR] Failed to allocate memory to buffer.\n");
		return NULL;
	}

	fread(buffer, 1, length, pfile);
	fclose(pfile);
	return buffer;
}

bool startswith(char *p, char *q){
	return strncmp(p, q, strlen(q)) == 0;
}
