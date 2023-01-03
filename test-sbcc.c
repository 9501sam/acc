#include <stdio.h>
#include <assert.h>

#include "types.h"
#include "scanner.h"
#include "parser.h"
#include "set.h"

extern void hashmap_test(void);
extern void scanner_test(void);


int main(int argc, char **argv)
{
    // hashmap_test();
    scanner_test();

    return 0;
}
