// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//computer
#include "utils.h"






// ---------------- TOOLS ----------------

//argument check
char checkMode(char* modeString) {
	ulng modeStringLength = strlen(modeString);
	if(modeStringLength != 1 || (modeString[0] != 'k' && modeString[0] != 'u')) {
		fprintf(stderr, "UC2HC: Invalid mode '%s' given (only 'k' and 'u' allowed)", modeString);
		exit(EXIT_FAILURE);
	}

	//mode is correct, return targetted character
	return modeString[0];
}
char* compile(char mode, char* UCContent) {

	//set mode flag to apply
	uint modeFlag = 0;
	if(mode == 'k') { modeFlag = 0x80000000; }

	//
	char* HCContent = malloc(5+1);
	HCContent[0] = 'E';
	HCContent[1] = 'M';
	HCContent[2] = 'P';
	HCContent[3] = 'T';
	HCContent[4] = 'Y';
	HCContent[5] = '\0';

	//operation successful
	return HCContent;
}





// ---------------- EXECUTION ----------------

//main
int main(int argc, char** argv) {

	//args
	if(argc <= 1) {
		fputs("UC2HC: Missing first argument <mode>.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if(argc <= 2) {
		fputs("UC2HC: Missing second argument <filename>.\n", stderr);
		exit(EXIT_FAILURE);
	}

	//check mode
	char mode = checkMode(argv[1]);

	//check and read input file
	char* inputFilename       = argv[2];
	ulng  inputFilenameLength = strlen(inputFilename);
	if(
		inputFilenameLength < 3                     ||
		inputFilename[inputFilenameLength-3] != '.' ||
		inputFilename[inputFilenameLength-2] != 'u' ||
		inputFilename[inputFilenameLength-1] != 'c'
	) {
		fprintf(stderr, "UC2HC: Incorrect file extension in input file '%s' ('.uc' expected).", inputFilename);
		exit(EXIT_FAILURE);
	}
	char* UCContent = readFile(inputFilename);

	//compile from UC to HC
	char* HCContent = compile(mode, UCContent);
	free(UCContent);

	//write out HC file
	inputFilename[inputFilenameLength-2] = 'h'; //replace ".uc" by ".hc"
	writeFile(inputFilename, HCContent);
	free(HCContent);

	//nothing went wrong
	return EXIT_SUCCESS;
}
