#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

token_t *scanning(char *path);

#endif /* __SCANNER_H__ */
