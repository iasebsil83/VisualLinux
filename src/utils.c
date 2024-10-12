// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//own header
#include "utils.h"






// ---------------- TOOLS ----------------

//default tools...
char* readFile(char* filename) {

	//read file and get its content size
	FILE* f = fopen(filename, "r");
	fseek(f, 0UL, SEEK_END);
	ulng fileSize = ftell(f);

	//read all
	fseek(f, 0UL, SEEK_SET);
	char* content = malloc((fileSize+1) * sizeof(char));
	ulng readBytes = fread(content, 1, fileSize, f);

	//close
	fclose(f);
	free(f);

	//error cases
	if(readBytes != fileSize) {
		fprintf(stderr, "Utils: Unable to read file \"%s\" properly.\n", filename);
		exit(EXIT_FAILURE);
	}

	//no error
	content[fileSize] = '\0'; //don't forget the terminating '\x00'
	return content;
}

void writeFile(char* filename, char* content) {
	ulng contentLength = strlen(content);

	//read file and get its content size
	FILE* f = fopen(filename, "w");
	ulng writtenBytes = fwrite(content, 1, contentLength, f);

	//close
	fclose(f);
	free(f);

	//error cases
	if(writtenBytes != contentLength) {
		fprintf(stderr, "Utils: Problems occured when writting %llu bytes to file '%s', you may not have missing parts.", contentLength, filename);
		exit(EXIT_FAILURE);
	}
}
