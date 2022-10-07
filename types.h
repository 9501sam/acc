#ifndef __TYPES_H__
#define __TYPES_H__

///*** symbol table ***///
#define TABLE_SIZE 26
typedef enum {NOT_USED, INTEGER, FLOAT} sym_type;
extern sym_type table[TABLE_SIZE];

///*** token ***///
typedef struct {
    enum {FLOATDCL, INTDCL, PRINT, ID, ASSIGN, PLUS, MINUS, INUM, FNUM} type;
    union {
        char ch; // ID
        char *str; // INUM, FNUM
    } val;
} token_t;

///*** token ***///
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
