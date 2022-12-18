#include "types.h"
#include "scanner.h"

char** extract_file(char *path, int *tot_line){
	FILE *pfile = fopen(path, "r");
	fpos_t position;

	int mx_line = 100, len = 0;
	char c;
	char **arr = (char**)malloc(mx_line * sizeof(char*));

	fgetpos(pfile, &position);
	do{
		// reallocate array size
		if (mx_line / 2 < *tot_line){
			mx_line *= 2;
			arr = (char**)realloc(arr, mx_line * sizeof(char*));
		}

		c = fgetc(pfile);
		len++;

		// assign line to array when reach '\n' or eof
		if (c == '\n' || c == EOF){
			if (len > 1){
				fsetpos(pfile, &position);
				arr[(*tot_line)] = (char*)calloc(len, sizeof(char));
				fgets(arr[(*tot_line)], len, pfile);
				fgetc(pfile); // skip '\n'
				(*tot_line)++;
			}
			len = 0;
			fgetpos(pfile, &position);
		}
	} while (c != EOF);
	return arr;
}

token_t *scanning(char *path)
{
    // TODO
	int tot_line = 0, cur_line = 0;
	int tot_char = 0, cur_char = 0;
	char **input = extract_file(path, &tot_line);
    return NULL;
}

///*** test ***///
void test_extract_file(){
    printf("======test extract_file======\n");
	int tot_line = 0;
	char **input;
	input = extract_file("scanner-input.txt", &tot_line);

	printf("Total line: %d\n", tot_line);
	assert(tot_line == 5);
	
	printf("[12345] == [%s]\n", input[0]);
	assert(memcmp("12345", input[0], strlen(input[0])) == 0);
	printf("[abc] == [%s]\n", input[1]);
	assert(memcmp("abc", input[1], strlen(input[1])) == 0);
	printf("[12] == [%s]\n", input[2]);
	assert(memcmp("12", input[2], strlen(input[2])) == 0);
	printf("[zxc] == [%s]\n", input[3]);
	assert(memcmp("zxc", input[3], strlen(input[3])) == 0);
	printf("[1222121] == [%s]\n", input[4]);
	assert(memcmp("1222121", input[4], strlen(input[4])) == 0);
}
