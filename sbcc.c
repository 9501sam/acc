#include <stdio.h>

#include "types.h"
#include "scanner.h"
#include "parser.h"

bool file_exist(char *path){
	FILE *pfile = fopen(path, "r");
	bool is_exist = true;
	if (pfile == NULL)
		is_exist = false;
	fclose(path);
	return is_exist;
}

int main(int argc, char **argv)
{

	// check if there is any arguments
	if (argc == 0){
		fprintf(stderr, "Doesn't input any file\n");
		return -1;
	}

	// for loop handle multiple file
	for (int i = 0; i < argc; i++){
		if (file_exist(argv[i]))
			continue;
		scanning(argv[i]);
	}

    return 0;
}
