// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//own header
#include "computer.h"






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
	free(f);

	//error cases
	if(readBytes != fileSize) {
		fprintf(stderr, "VisualLinux: Unable to read file \"%s\" properly.\n", filename);
		exit(EXIT_FAILURE);
	}

	//no error
	content[fileSize] = '\0'; //don't forget the terminating '\x00'
	return content;
}

//instantiate a new computer
cpt* newComputer(char* storageDir) {
	cpt* c = malloc(sizeof(cpt));
	c->storageDir = storageDir;

	//instructions
	c->instructions = malloc(CPT__INSTRUCTIONS_LENGTH * sizeof(instruction*));
	for(ulng i=0ULL; i < CPT__INSTRUCTIONS_LENGTH; i++) { c->instructions[i] = NULL; }
	c->currentInstructionIndex = 0ULL;

	//cpu mems
	c->cpuMems = malloc(CPT__CPUMEMS_LENGTH * sizeof(cpuMem*));
	for(ulng i=0ULL; i < CPT__CPUMEMS_LENGTH; i++) {
		c->cpuMems[i] = malloc(sizeof(cpuMem));
		cpuMem* cm    = c->cpuMems[i]; //just a local shortcut

		//registers
		cm->registers = malloc(CPT__CPUMEM_REGISTERS_LENGTH * sizeof(short));
		for(ulng j=0ULL; j < CPT__CPUMEM_REGISTERS_LENGTH; j++) { cm->registers[j] = 0; }

		//stack
		cm->stack = malloc(CPT__CPUMEM_STACK_LENGTH * sizeof(short));
		for(ulng j=0ULL; j < CPT__CPUMEM_STACK_LENGTH; j++) { cm->stack[j] = 0; }
	}
	c->currentCpuMemIndex = 0ULL;

	//ram
	c->ram = malloc(CPT__RAM_LENGTH * sizeof(short));
	for(ulng i=0ULL; i < CPT__RAM_LENGTH; i++) { c->ram[i] = 0; }

	//computer initialized
	return c;
}






// ---------------- EXECUTION ----------------

//parse HC code line
void loadHCContent(char* content, instruction** instructions, ulng startingIndex) {

	//check length integrity
	ulng contentLen = strlen(content);
	if(contentLen%CPT__SINGLE_INSTRUCTION_LENGTH != 0) {
		fprintf(
			stderr,
			"VisualLinux: Invalid number of bytes in HC content (multiples of %llu required, %llu found).\n",
			CPT__SINGLE_INSTRUCTION_LENGTH,
			contentLen
		);
		exit(EXIT_FAILURE);
	}

	//check instruction number
	ulng newInstructionsLen = contentLen / CPT__SINGLE_INSTRUCTION_LENGTH;
	ulng remainingSlots = CPT__INSTRUCTIONS_LENGTH - startingIndex;
	if(newInstructionsLen > remainingSlots) {
		fprintf(
			stderr,
			"VisualLinux: Too much instructions taken from HC content (%llu slots remaining, %llu to load).\n",
			remainingSlots,
			newInstructionsLen
		);
		exit(EXIT_FAILURE);
	}

	//for each instruction to load
	ulng instructionsIndex = startingIndex;
	for(ulng i=0ULL; i < newInstructionsLen; i++) {

		//allocation + lineNbr
		instructions[instructionsIndex]          = malloc(sizeof(instruction)); //instructions[i] should be NULL
		instructions[instructionsIndex]->lineNbr = instructionsIndex;

		//text field
		instructions[instructionsIndex]->text = malloc(CPT__SINGLE_INSTRUCTION_LENGTH * sizeof(char));
		ulng  textOffset = CPT__SINGLE_INSTRUCTION_LENGTH * i;
		char* text       = instructions[instructionsIndex]->text;
		int   t          = 0;
		for(; t < CPT__SINGLE_INSTRUCTION_LENGTH - 1; t++) { text[t] = content[textOffset+t]; } //no need to get terminating LINE FEED
		text[t] = '\0'; //better replace it by the string termination character

		//get instruction mode
		char mode = content[textOffset];
		if(mode != 'k' && mode != 'u') {
			fprintf(stderr, "VisualLinux: Invalid mode '%c' in instruction ('k' or 'u' expected).\n", mode);
			exit(EXIT_FAILURE);
		}
		instructions[instructionsIndex]->mode = mode;

		//get instruction name
		char name = content[textOffset+1];
		if(name != 'P' && name != 'A' && name != 'R' && name != 'W') {
			fprintf(stderr, "VisualLinux: Invalid name '%c' in instruction ('P', 'A', 'R' or 'W' expected).\n", name);
			exit(EXIT_FAILURE);
		}
		instructions[instructionsIndex]->name = name;

		//get parameter type
		char pType = content[textOffset+2];
		if(pType != 'R' && pType != 'V') {
			fprintf(stderr, "VisualLinux: Invalid parameter type '%c' in instruction ('R' or 'V' expected).\n", pType);
			exit(EXIT_FAILURE);
		}
		instructions[instructionsIndex]->pType = pType;

		//get parameter value
		instructions[instructionsIndex]->pValue = 0;
		for(int j=0; j < 4; j++) {
			short current = content[textOffset+3+j]; //IMPORTANT: must be at least a short !!!

			//decimal character
			if(current >= '0' && current <= '9') { instructions[instructionsIndex]->pValue += (current - '0') << (3-j); }

			//alpha character
			else if(current >= 'a' && current <= 'f') { instructions[instructionsIndex]->pValue += (current - 'a') << (3-j); }

			//error cases
			else {
				fprintf(
					stderr,
					"VisualLinux: Invalid parameter value '%c%c%c%c' in instruction (only hex characters expected).\n",
					content[textOffset+3], content[textOffset+4],
					content[textOffset+5], content[textOffset+6]
				);
				exit(EXIT_FAILURE);
			}
		}

		//follow i evolution
		instructionsIndex++;
	}
}

//operate CPU
void operateCPU(cpt* computer) {
	instruction* currentInstruction = computer->instructions[computer->currentInstructionIndex];
	switch(currentInstruction->name){

		//print
		case 'P':
			puts("Performing HW operation PRINT with parameter.");
		break;

		//Move buffer read
		case 'R':
			if(currentInstruction->pType == 'V') {
				fprintf(stderr, "VisualLinux: Invalid HC instruction \"%s\" (R with VALUE parameter).", currentInstruction->text);
				exit(EXIT_FAILURE);
			}
			puts("Performing HW operation MOVE BUFFER READ.");
		break;

		//Move buffer write
		case 'W':
			puts("Performing HW operation MOVE BUFFER WRITE.");
		break;

		//add
		case 'A':
			puts("Performing HW operation ADD.");
		break;

		//undefined instruction
		default:
			fprintf(
				stderr,
				"VisualLinux: Invalid HC instruction '%c' detected at line %llu : \"%s\".\n",
				currentInstruction->name,
				computer->currentInstructionIndex,
				currentInstruction->text
			);
			exit(EXIT_FAILURE);
		break;
	}
}

//kernel
void loadKernel(cpt* computer) {

	//format kernel program filename
	ulng  kernelFilenameLen = strlen(computer->storageDir) + 11ULL; //second item is strlen("/kernel.hc" + '\0')
	char* kernelFilename = malloc(kernelFilenameLen);
	sprintf(kernelFilename, "%s/kernel.hc", computer->storageDir);

	//load content of kernel program
	loadHCContent(
		readFile(kernelFilename),
		computer->instructions,
		0ULL                    //load instructions from the beginning
	);
	free(kernelFilename);
}
