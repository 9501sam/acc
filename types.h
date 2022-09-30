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

// ast
typedef struct {
} ast_t;

#endif /* __TYPES_H__ */
