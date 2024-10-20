// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//own header
#include "utils.h"






// ---------------- TOOLS ----------------

//endianness
bool isBigEndian() {
	static const int i = 1;
	return ((*(char*)&i) == 0);
}

//IO
void freeBuffer(buffer* content) { free(content->data); free(content); }

buffer* readFile(char* filename) {
	buffer* content = malloc(sizeof(buffer));

	//read file and get its content size
	FILE* f = fopen(filename, "r");
	fseek(f, 0UL, SEEK_END);
	content->length = ftell(f);

	//read all
	fseek(f, 0UL, SEEK_SET);
	content->data  = malloc(content->length * sizeof(char));
	ulng readBytes = fread(content->data, 1, content->length, f);
	fclose(f);

	//error cases
	if(readBytes != content->length) {
		fprintf(stderr, "Utils: Unable to read file \"%s\" properly.\n", filename);
		exit(EXIT_FAILURE);
	}

	//no error
	return content;
}

void writeFile(char* filename, buffer* content) {

	//read file and get its content size
	FILE* f = fopen(filename, "w");
	ulng writtenBytes = fwrite(content->data, 1, content->length, f);
	fclose(f);

	//error cases
	if(writtenBytes != content->length) {
		fprintf(stderr, "Utils: Problems occured when writting %llu bytes to file '%s', you may not have missing parts.", content->length, filename);
		exit(EXIT_FAILURE);
	}
}
