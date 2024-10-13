// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//computer
#include "computer.h"

//tools
#include "utils.h"






// ---------------- DECLARATIONS ----------------

//limits
#define MAX_HC_LENGTH CPT__RAM_LENGTH

//symbol resolution
#define DATA_TABLE_LENGTH      32ULL
#define LABEL_TABLE_LENGTH     32ULL
#define MAX_SYMBOL_NAME_LENGTH 32ULL
typedef struct {
	char* name;
	ulng  length;  // <- This field allows us to directly make name point to locations in code
	ushr  address; //    without having to make local copies. We only rely on it to know where to stop.
} sym;






// ---------------- TOOLS ----------------

//conversions
char hexToHalfByte(ulng lineNbr, char h) { //1 hex character  => corresponding byte value (only the 4 LSb)
	if(h >= '0' && h <= '9') { return h - '0';          }
	if(h >= 'a' && h <= 'f') { return h - 'a' + '\x0a'; }
	fprintf(stderr, "UC2HC: [line %llu] Invalid hex character '%c' (%02x).\n", lineNbr, h, (int)h);
	exit(EXIT_FAILURE);
}
char biHexToByte(ulng lineNbr, char h1, char h2) { //2 hex characters => corresponding byte value
	return (hexToHalfByte(lineNbr, h1) << 4) | hexToHalfByte(lineNbr, h2);
}

//argument check
char checkMode(char* modeString) {
	ulng modeStringLength = strlen(modeString);
	if(modeStringLength != 1 || (modeString[0] != 'k' && modeString[0] != 'u')) {
		fprintf(stderr, "UC2HC: Invalid mode '%s' given (only 'k' and 'u' allowed).\n", modeString);
		exit(EXIT_FAILURE);
	}

	//mode is correct, return targetted character
	return modeString[0];
}
void checkHCIndex(ulng lineNbr, ulng index) {
	if(index >= MAX_HC_LENGTH) {
		fprintf(stderr, "UC2HC: [line %llu] Too much HC to process (maximum %i words allowed).\n", lineNbr, MAX_HC_LENGTH);
		exit(EXIT_FAILURE);
	}
}
ushr getPTypeFlag(ulng lineNbr, char c) {
	if(c == 'r') { return CPT__INSTRUCTION_PTYP_REGISTER; }
	if(c == 'v') { return CPT__INSTRUCTION_PTYP_VALUE;    }
	fprintf(stderr, "UC2HC: [line %llu] Invalid parameter type '%c' given ('r' or 'v' expected).\n", lineNbr, c);
	exit(EXIT_FAILURE);
}






// ---------------- SYMBOLS ----------------

//check [a-zA-Z_]
void checkSymbolName(ulng lineNbr, char* text, ulng textLength) {

	//check length
	if(textLength > MAX_SYMBOL_NAME_LENGTH) {
		fprintf(stderr, "UC2HC: [line %llu] Too much character in symbol name (maximum %llu allowed, %llu given).\n", lineNbr, MAX_SYMBOL_NAME_LENGTH, textLength);
		exit(EXIT_FAILURE);
	}

	//check each individual character
	for(ulng i=0; i < textLength; i++) {
		char t = text[i];
		if( t != '_' && (t < 'A' || t > 'z' || (t > 'Z' && t < 'a')) ) {
			fprintf(stderr, "UC2HC: [line %llu] Invalid character '%c' (\\x%02x) given in symbol name.\n", lineNbr, t, (int)t);
			exit(EXIT_FAILURE);
		}
	}
}

//get
sym* getSymbolOrNULL(ulng lineNbr, sym* tab, ulng tabLength, char* text, ulng textLength) {
	for(ulng s=0ULL; s < tabLength; s++) {
		char* name = tab[s].name;
		if(name != NULL) {

			//check symbol names length
			if(tab[s].length != textLength) { continue; }

			//check each character
			for(ulng c=0ULL; c < textLength; c++) { if(name[c] != text[c]) { continue; } }

			//match !
			return tab + s;
		}
	}

	//no symbol found => return NULL
	return NULL;
}
sym* getSymbol(ulng lineNbr, sym* tab, ulng tabLength, char* text, ulng textLength) { //exit on error
	sym* s = getSymbolOrNULL(lineNbr, tab, tabLength, text, textLength);
	if(s == NULL) {
		fprintf(stderr, "UC2HC: [line %llu] Unable to find symbol '%s' in its corresponding table.\n", lineNbr, text);
		exit(EXIT_FAILURE);
	}
	return s;
}

//add (set)
void addSymbol(ulng lineNbr, sym* tab, ulng tabLength, char* text, ulng textLength, ushr address) {

	//check name validity first
	checkSymbolName(lineNbr, text, textLength);

	//check that we don't already have a symbol with that name
	sym* s = getSymbolOrNULL(lineNbr, tab, tabLength, text, textLength);
	if(s != NULL) {
		fprintf(stderr, "UC2HC: [line %llu] Symbol with name '", lineNbr);
		for(ulng c=0ULL; c < textLength; c++) { fputc(text[c], stderr); }
		fputs("' already exists in its corresponding table.\n", stderr);
		exit(EXIT_FAILURE);
	}

	//check for the first NULL symbol to add our new one
	for(ulng s=0ULL; s < tabLength; s++) {
		if(tab[s].name == NULL) {
			tab[s].name    = text;
			tab[s].length  = textLength;
			tab[s].address = address;
			return;
		}
	}

	//could not add symbol => limit reached
	fprintf(stderr, "UC2HC: [line %llu] Could not add symbol '%s' to its table, limit reached.\n", lineNbr, text);
	exit(EXIT_FAILURE);
}






// ---------------- COMPILATION ----------------

//shortcuts
void checkNotEnoughCharacters(ulng lineNbr, ulng given, ulng required, const char* instructionType) {
	if(given < required) {
		fprintf(
			stderr,   "UC2HC: [line %llu] Not enough characters to process %s instruction (%llu required, %llu given).\n",
			lineNbr,  instructionType,
			required, given
		);
		exit(EXIT_FAILURE);
	}
}
ulng increaseUnderLimit(ulng lineNbr, ulng value, ulng amount, ulng limit, const char* instructionType) {
	checkNotEnoughCharacters(lineNbr, limit-1ULL, value+amount, instructionType);
	return value + amount;
}
void checkSpaceSeparator(ulng lineNbr, char c) {
	if(c != ' ') {
		fprintf(stderr, "UC2HC: [line %llu] Missing space separator.\n", lineNbr);
		exit(EXIT_FAILURE);
	}
}

//main part
char* compile(char mode, char* UCContent, ulng* resultLength) { //resultLength is an output value (should NOT be set to NULL)
	ulng UCContentLength = strlen(UCContent);

	//set mode flag to apply
	ushr modeFlag = CPT__INSTRUCTION_MODE_USER;
	if(mode == 'k') { modeFlag = CPT__INSTRUCTION_MODE_KERNEL; }

	//allocate maximum program size
	ushr* HCContent = malloc(MAX_HC_LENGTH*sizeof(ushr)); //only the necessary bytes will be written

	//create symbol tables
	sym* datTab = malloc(DATA_TABLE_LENGTH * sizeof(sym));
	for(ulng s=0ULL; s < DATA_TABLE_LENGTH; s++) { datTab[s].name = NULL; }
	sym* lblTab = malloc(LABEL_TABLE_LENGTH * sizeof(sym));
	for(ulng l=0ULL; l < LABEL_TABLE_LENGTH; l++) { lblTab[l].name = NULL; }



	// ==== HC format constitution ====
	//
	//  To have a static data space in our program, we need to have a data section where things are stored
	//  as well as an execution section to hold programs instructions.
	//  To do so, the strategy adopted here is to make this structure into our HC content :
	//      - DATA SECTION JUMPER
	//      - DATA SECTION
	//      - EXECUTION SECTION
	//
	//  Our HC content starts by a JUMP instruction that goes over the data section of the program.
	//  That means we must know the size of the data section to then set the correct amount of instructions to JUMP.
	//  Also, because the execution can use static data (using data item symbols), we must first know where to
	//  find that data so where to look inside the data section.
	//
	//  For those reasons we create our HC content this way :
	//    - Create the DATA SECTION (skipping the beginning jumper instruction for the moment)
	//    - Set the beginning JUMPER (using the resulting size of the data section we just made)
	//    - Finally create EXECUTION SECTION (that can refer to elements previously stored in data section)



	//STEP 1: CREATING DATA SECTION (first read of UC content)

	//start HC content 2 words after the beginning (2 words = 1 instruction => for the data section JUMPER)
	ulng currentHCIndex = 2ULL;

	//read input content
	ulng currentLine_startIndex = 0ULL;
	ulng lineNbr                = 1ULL;
	for(ulng c=0ULL; c < UCContentLength; c++) {
		if(UCContent[c] == '\n') {
			lineNbr++;

			//take care of D instructions only (1st byte)
			ulng lineLength = c - currentLine_startIndex;
			if(lineLength > 0 && UCContent[currentLine_startIndex] == 'D') {
				checkNotEnoughCharacters(lineNbr, lineLength, 3, "DATA");

				//check 2nd & 3rd byte
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA"); //<=> currentLine_startIndex++ with sign security
				checkSpaceSeparator(lineNbr, UCContent[currentLine_startIndex]);
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA");
				char dataTypeIdentifier = UCContent[currentLine_startIndex];
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA");

				//extract full data depending on the data type
				ulng paramLength = c - currentLine_startIndex;
				ushr pValue;
				ulng strLength;
				switch(dataTypeIdentifier) {

					//word (2 bytes)
					case 'w':

						//process value
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "DATA WORD");
						pValue =
							(biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex  ],
								UCContent[currentLine_startIndex+1]
							) << 8) | biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex+2],
								UCContent[currentLine_startIndex+3]
							);

						//another space separator
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 4ULL, c, "DATA WORD SYMBOL NAME SEPARATOR");
						checkSpaceSeparator(lineNbr, UCContent[currentLine_startIndex]);

						//process symbol name
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA WORD");
						paramLength -= 5;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "DATA WORD SYMBOL NAME");
						addSymbol(
							lineNbr,                          datTab,      DATA_TABLE_LENGTH,
							UCContent+currentLine_startIndex, paramLength, currentHCIndex
						);

						//write raw word in HC content
						checkHCIndex(lineNbr, currentHCIndex);
						HCContent[currentHCIndex++] = pValue;
					break;

					//text strings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TODO
					case '"':
						fprintf(stderr, "UC2HC: [line %llu] String data items not implemented yet.\n", lineNbr);
						exit(EXIT_FAILURE);
					break;

					//unknown identifier
					default:
						fprintf(stderr, "UC2HC: [line %llu] Unknown identifier '%c' in DATA instruction.\n", lineNbr, UCContent[currentLine_startIndex]);
						exit(EXIT_FAILURE);
				}
			}

			//move forward to the next instruction
			currentLine_startIndex = c+1;
		}
	}



	//STEP 2: SET DATA SECTION JUMPER

	//set custom jumper as first instruction of the program
	HCContent[0] = modeFlag | CPT__INSTRUCTION_PTYP_VALUE | CPT__INSTRUCTION_ID_JMP;
	HCContent[1] = currentHCIndex;



	//STEP 3: CREATE EXECUTION SECTION (second read of UC content)

	//reading input content
	currentLine_startIndex = 0ULL;
	lineNbr                = 1ULL;
	for(ulng c=0ULL; c < UCContentLength; c++) {
		if(UCContent[c] == '\n') {
			lineNbr++;

			//empty line => skip it
			if(currentLine_startIndex == c) { currentLine_startIndex = c+1; continue; }

			//check we have at least the minimum number of characters to analyse
			ulng lineLength = c - currentLine_startIndex;
			checkNotEnoughCharacters(lineNbr, lineLength, 3, "UC");

			//check first 2 bytes
			char instructionIdentifier = UCContent[currentLine_startIndex];
			currentLine_startIndex     = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "UC");
			checkSpaceSeparator(lineNbr, UCContent[currentLine_startIndex]);
			currentLine_startIndex     = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "UC");

			//redirections
			ulng  paramLength = c - currentLine_startIndex;
			ushr  pTypeFlag, pValue;
			switch(instructionIdentifier) {

				//comment or Data (do nothing)
				case ';': case 'D': currentLine_startIndex = c+1; continue; break;

				//label
				case 'L':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "LABEL");
					addSymbol(
						lineNbr,                          lblTab,      LABEL_TABLE_LENGTH,
						UCContent+currentLine_startIndex, paramLength, currentHCIndex
					);
				break;

				//print
				case 'P':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "PRINT");

					//case 1: resolve data symbol
					if(UCContent[currentLine_startIndex] == 's') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "PRINT DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "PRINT DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                          datTab,     DATA_TABLE_LENGTH,
							UCContent+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, UCContent[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "PRINT DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "PRINT DATA VALUE");
						pValue =
							(biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex  ],
								UCContent[currentLine_startIndex+1]
							) << 8) | biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex+2],
								UCContent[currentLine_startIndex+3]
							);
					}

					//add HC instruction
					checkHCIndex(lineNbr, currentHCIndex);
					HCContent[currentHCIndex++] = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_PRT;
					checkHCIndex(lineNbr, currentHCIndex);
					HCContent[currentHCIndex++] = pValue;
				break;

				//jump
				case 'J':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP");

					//case 1: resolve data symbol
					if(UCContent[currentLine_startIndex] == 's') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                          datTab,     DATA_TABLE_LENGTH,
							UCContent+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: resolve label symbol
					else if(UCContent[currentLine_startIndex] == 'l') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP LABEL SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP LABEL SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                          lblTab,     LABEL_TABLE_LENGTH,
							UCContent+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, UCContent[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "JUMP DATA VALUE");
						pValue =
							(biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex  ],
								UCContent[currentLine_startIndex+1]
							) << 8) | biHexToByte(
								lineNbr,
								UCContent[currentLine_startIndex+2],
								UCContent[currentLine_startIndex+3]
							);
					}

					//add HC instruction
					checkHCIndex(lineNbr, currentHCIndex);
					HCContent[currentHCIndex++] = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_JMP;
					checkHCIndex(lineNbr, currentHCIndex);
					HCContent[currentHCIndex++] = pValue;
				break;

				//unknown instruction
				default:
					fprintf(stderr, "UC2HC: [line %llu] Unknown UC instruction starting from '%c'.\n", lineNbr, instructionIdentifier);
					exit(EXIT_FAILURE);
			}

			//everything went fine with this line, move forward to the next one
			currentLine_startIndex = c+1;
		}
	}

	//free sym tabs: no need to free names stored inside, they are only pointers to locations in UCContent
	free(datTab);
	free(lblTab);

	//set length of HCContent
	resultLength[0] = currentHCIndex * sizeof(ushr); //number of bytes

	//operation successful
	return (char*)HCContent;
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
		fprintf(stderr, "UC2HC: Incorrect file extension in input file '%s' ('.uc' expected).\n", inputFilename);
		exit(EXIT_FAILURE);
	}
	char* UCContent = readFile(inputFilename);

	//compile from UC to HC
	ulng  HCContentLength = 0ULL;
	char* HCContent       = compile(mode, UCContent, &HCContentLength);
	free(UCContent);

	//write out HC file
	inputFilename[inputFilenameLength-2] = 'h'; //replace ".uc" by ".hc"
	writeFile(inputFilename, HCContent, HCContentLength);
	free(HCContent);

	//nothing went wrong
	return EXIT_SUCCESS;
}
