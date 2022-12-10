#include <stdio.h>

#include "types.h"
#include "scanner.h"
#include "parser.h"

int main(int argc, char **argv)
{
    // TODO: read from file argv[1]

    token_t *tokens = scanning("f b i a a = 5 b = a + 3.2 p b");
    ast_t *ast = parsing(tokens);

    return 0;
}
// Test
