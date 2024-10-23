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
	if(index >= 2ULL*MAX_HC_LENGTH) {
		fprintf(stderr, "UC2HC: [line %llu] Too much HC to process (maximum %i words = %llu bytes allowed).\n", lineNbr, MAX_HC_LENGTH, 2ULL*MAX_HC_LENGTH);
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

ushr getWord(ulng lineNbr, char* text, ushr currentInstructionPos) {

	//special value
	if(text[0] == '%' && text[1] == '%' && text[2] == '%' && text[3] == '%') { return currentInstructionPos; }

	//regular value
	char hv1 = 0x00ff & biHexToByte(lineNbr, text[0], text[1]);
	char hv2 = 0x00ff & biHexToByte(lineNbr, text[2], text[3]);
	return hv1 << 8 | hv2;
}

//main part
buffer* compile(char mode, buffer* UCContent) {

	//set mode flag to apply
	ushr modeFlag = CPT__INSTRUCTION_MODE_USER;
	if(mode == 'k') { modeFlag = CPT__INSTRUCTION_MODE_KERNEL; }

	//allocate maximum program size
	buffer* HCContent = malloc(sizeof(buffer));
	HCContent->data   = malloc(MAX_HC_LENGTH*sizeof(ushr)); //only the necessary bytes will be written

	//create symbol tables
	sym* datTab = malloc(DATA_TABLE_LENGTH * sizeof(sym));
	for(ulng s=0ULL; s < DATA_TABLE_LENGTH; s++) { datTab[s].name = NULL; }
	sym* lblTab = malloc(LABEL_TABLE_LENGTH * sizeof(sym));
	for(ulng l=0ULL; l < LABEL_TABLE_LENGTH; l++) { lblTab[l].name = NULL; }

	//optimization (and also shortcuts btw)
	char* input  = UCContent->data;
	char* output = HCContent->data;



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
	ulng currentHCIndex = 4ULL;

	//read input content
	ulng currentLine_startIndex = 0ULL;
	ulng lineNbr                = 1ULL;
	ulng stringFirstIndex;
	ushr pTypeFlag, header, pValue;
	bool escaped, foundEnd;
	for(ulng c=0ULL; c < UCContent->length; c++) {
		if(input[c] == '\n') {
			lineNbr++;

			//take care of D instructions only (1st byte)
			ulng lineLength = c - currentLine_startIndex;
			if(lineLength > 0 && input[currentLine_startIndex] == 'D') {
				checkNotEnoughCharacters(lineNbr, lineLength, 3, "DATA");

				//check 2nd & 3rd byte
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA"); //<=> currentLine_startIndex++ with sign security
				checkSpaceSeparator(lineNbr, input[currentLine_startIndex]);
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA");
				char dataTypeIdentifier = input[currentLine_startIndex];
				currentLine_startIndex  = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA");

				//extract full data depending on the data type
				ulng paramLength = c - currentLine_startIndex;
				ulng strLength;
				switch(dataTypeIdentifier) {

					//word (2 bytes)
					case 'w':

						//process value
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "DATA WORD");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);

						//another space separator
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 4ULL, c, "DATA WORD SYMBOL NAME SEPARATOR");
						checkSpaceSeparator(lineNbr, input[currentLine_startIndex]);

						//process symbol name
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA WORD");
						paramLength -= 5;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "DATA WORD SYMBOL NAME");
						addSymbol(
							lineNbr,                      datTab,      DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength, currentHCIndex
						);

						//write raw word in HC content !!!WARNING LITTLE ENDIAN BEHAVIOR
						checkHCIndex(lineNbr, currentHCIndex);
						output[currentHCIndex++] = pValue & 0x00ff;
						checkHCIndex(lineNbr, currentHCIndex);
						output[currentHCIndex++] = pValue >> 8;
					break;

					//text strings
					case '"':
						stringFirstIndex = currentLine_startIndex;
						escaped = false; foundEnd = false;
						for(ulng p=0ULL; p < paramLength; p++) {

							//end of string
							if(escaped) { escaped = false; }
							else if(input[currentLine_startIndex] == '\\') { escaped = true; continue; }
							else if(input[currentLine_startIndex] ==  '"') { foundEnd = false; break;  }

							//inside string => store it
							checkHCIndex(lineNbr, currentHCIndex);
							output[currentHCIndex++] = input[currentLine_startIndex];
							currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA STRING");
							paramLength--;
						}

						//no end delimiter found
						if(!foundEnd) {
							fprintf(stderr, "UC2HC: [line %llu] Missing string terminator '\"'.\n", lineNbr);
							exit(EXIT_FAILURE);
						}

						//adding string terminator
						checkHCIndex(lineNbr, currentHCIndex);
						output[currentHCIndex++] = '\x00';

						//another space separator
						checkSpaceSeparator(lineNbr, input[currentLine_startIndex]);

						//process symbol name
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "DATA STRING");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "DATA STRING SYMBOL NAME");
						addSymbol(
							lineNbr,                      datTab,      DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength, stringFirstIndex
						);
					break;

					//unknown identifier
					default:
						fprintf(stderr, "UC2HC: [line %llu] Unknown identifier '%c' in DATA instruction.\n", lineNbr, input[currentLine_startIndex]);
						exit(EXIT_FAILURE);
				}
			}

			//move forward to the next instruction
			currentLine_startIndex = c+1;
		}
	}



	//STEP 2: SET DATA SECTION JUMPER

	//set custom jumper as first instruction of the program !!!WARNING LITTLE ENDIAN BEHAVIOR
	header    = modeFlag | CPT__INSTRUCTION_PTYP_VALUE | CPT__INSTRUCTION_ID_JMP;
	output[0] = header & 0x00ff;
	output[1] = header >> 8;
	output[2] = currentHCIndex & 0x00ff;
	output[3] = currentHCIndex >> 8;



	//STEP 3: CREATE EXECUTION SECTION (second read of UC content)

	//reading input content
	currentLine_startIndex = 0ULL;
	lineNbr                = 1ULL;
	for(ulng c=0ULL; c < UCContent->length; c++) {
		if(input[c] == '\n') {
			lineNbr++;

			//empty line => skip it
			if(currentLine_startIndex == c) { currentLine_startIndex = c+1; continue; }

			//check we have at least the minimum number of characters to analyse
			ulng lineLength = c - currentLine_startIndex;
			checkNotEnoughCharacters(lineNbr, lineLength, 3, "UC");

			//check first 2 bytes
			char instructionIdentifier = input[currentLine_startIndex];
			currentLine_startIndex     = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "UC");

			//first space separator
			checkSpaceSeparator(lineNbr, input[currentLine_startIndex]);
			currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "UC");

			//redirections
			ulng  paramLength = c - currentLine_startIndex;
			switch(instructionIdentifier) {

				//comment or Data (do nothing)
				case ';': case 'D': currentLine_startIndex = c+1; continue; break;



				//label
				case 'L':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "LABEL");
					addSymbol(
						lineNbr,                      lblTab,      LABEL_TABLE_LENGTH,
						input+currentLine_startIndex, paramLength, currentHCIndex
					);
				break;



				//add
				case 'A':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "ADD");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "ADD DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "ADD DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "ADD DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "ADD DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_ADD;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header >> 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue >> 8;
				break;



				//multiply
				case 'X':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "MULTIPLY");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "MULTIPLY DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "MULTIPLY DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "MULTIPLY DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "MULTIPLY DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_MUL;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header >> 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue >> 8;
				break;



				//read something into R0000
				case 'R':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "READ");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "READ DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "READ DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "READ DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "READ DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_REA;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header >> 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue >> 8;
				break;



				//write R0000 somewhere
				case 'W':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "WRITE");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "WRITE DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "WRITE DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "WRITE DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "WRITE DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_WRI;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header >> 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue >> 8;
				break;



				//print
				case 'P':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "PRINT");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "PRINT DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "PRINT DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "PRINT DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "PRINT DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_PRT;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header >> 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue >> 8;
				break;



				//jump
				case 'J':
					checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP");

					//case 1: resolve data symbol
					if(input[currentLine_startIndex] == 'd') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP DATA ITEM SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP DATA ITEM SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      datTab,     DATA_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: resolve label symbol
					else if(input[currentLine_startIndex] == 'l') {
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP LABEL SYMBOL NAME");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 1, "JUMP LABEL SYMBOL NAME"); //symbol name must have at least 1 character

						//set parameter type & value
						pTypeFlag = CPT__INSTRUCTION_PTYP_VALUE;
						pValue    = getSymbol(
							lineNbr,                      lblTab,     LABEL_TABLE_LENGTH,
							input+currentLine_startIndex, paramLength
						)->address;
					}

					//case 2: raw hex value (register or value)
					else {
						pTypeFlag = getPTypeFlag(lineNbr, input[currentLine_startIndex]);
						currentLine_startIndex = increaseUnderLimit(lineNbr, currentLine_startIndex, 1ULL, c, "JUMP DATA VALUE");
						paramLength--;
						checkNotEnoughCharacters(lineNbr, paramLength, 4, "JUMP DATA VALUE");
						pValue = getWord(lineNbr, input+currentLine_startIndex, currentHCIndex);
					}

					//add HC instruction !!!WARNING LITTLE ENDIAN BEHAVIOR
					header = modeFlag | pTypeFlag | CPT__INSTRUCTION_ID_JMP;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = header << 8;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue & 0x00ff;
					checkHCIndex(lineNbr, currentHCIndex);
					output[currentHCIndex++] = pValue << 8;
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

	//free sym tabs: no need to free names stored inside, they are only pointers to locations in input
	free(datTab);
	free(lblTab);

	//set length of HCContent
	HCContent->length = currentHCIndex * sizeof(ushr); //number of bytes

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

	//endianness
	if(isBigEndian()) {
		fputs("This program can't be executed on big endian machines.\n", stderr);
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
	buffer* UCContent = readFile(inputFilename);

	//compile from UC to HC
	buffer* HCContent = compile(mode, UCContent);
	freeBuffer(UCContent);

	//write out HC file
	inputFilename[inputFilenameLength-2] = 'h'; //replace ".uc" by ".hc"
	writeFile(inputFilename, HCContent);
	freeBuffer(HCContent);

	//nothing went wrong
	return EXIT_SUCCESS;
}
