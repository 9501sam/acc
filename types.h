#ifndef __TYPES_H__
#define __TYPES_H__

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

typedef struct _ast_node_t {
} ast_node_t;

typedef struct {
    ast_node_t *childs;
} ast_t;

#endif /* __TYPES_H__ */
