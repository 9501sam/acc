#include <stdio.h>

#include "types.h"
#include "scanner.h"
#include "parser.h"

bool file_exist(char *path){
	FILE *pfile = fopen(path, "r");
	bool is_exist = false;
	if (pfile != NULL)
		is_exist = true;
	fclose(path);
	return is_exist;

int main(int argc, char **argv)
{
    // TODO: read from file argv[1]

	if (argc == 0){
		fprintf("Doesn't input any file\n");
		return -1;
	}

	for (int i = 0; i < argc; i++){
		if (file_exist(argv[i]))
			continue;
		scanning(argv[i]);
	}

    return 0;
}
// Test
