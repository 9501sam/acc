#ifndef __TYPES_H__
#define __TYPES_H__

///*** symbol table ***///
/* never used
 * #define TABLE_SIZE 26
 * typedef enum {NOT_USED, INTEGER, FLOAT} sym_type;
 * extern sym_type table[TABLE_SIZE];
 */

///*** scanner token ***///
typedef enum {
	RESERVED_WORD,
	LIBRARY_NAME,
	IDENTIFIER,
	CHARACTER,
	NUMBER,
	POINTER,
	BRACKET,
	OPERATOR,
	COMPARATOR,
	ADDRESS,
	PUNCTUATION,
	FORMAT_SPECIFIER,
	PRINTED_TOKEN,
	COMMENT,
	UNDEFINED_TOKEN,
	SKIPPED_TOKEN
} token_type;
typedef struct {
	token_type type;
	char *str; //value
} token_t;

///*** parser token ***///
typedef enum {
    SYM_DECLARING,
    SYM_REFERENCING,
    COMPUTING,
    ASSIGNING,
    INT_CONSTING,
    FLOAT_CONSTING,
    PRINTING,
    CONVERTING
} ast_node_type;

typedef struct _ast_node_t {
    ast_node_type type;
    union {
        struct _ast_node_t *child;
        struct _ast_node_t *childs[2];
    };
} ast_node_t;

typedef struct {
    ast_node_t *childs;
} ast_t;

#endif /* __TYPES_H__ */
