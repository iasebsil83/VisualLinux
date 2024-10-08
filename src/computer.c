// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//own header
#include "computer.h"






// ---------------- EXECUTION ----------------

//default tools...
char* readFile(char* filename) {

	//read file and get its content size
	FILE* f = open(filename, "r");
	fseek(f, 0UL, SEEK_END);
	ulng fileSize = ftell(f);

	//read all
	fseek(f, 0UL, SEEK_SET);
	char* content = malloc((fileSize+1) * sizeof(char));
	ulng readBytes = fread(content, 1, fileSize, f);

	//error cases
	if(readBytes != fileSize) {
		fprintf("VisualLinux: Unable to read file \"%s\" properly.\n", filename);
		exit(EXIT_FAILURE);
	}

	//no error
	content[fileSize] = '\0'; //don't forget the terminating '\x00'
	return content;
}

//parse HC code line
instruction* parseHCfile(char* filename, ulng instructionsLenLimit, ulng lineNbrOffset) {

	//read file
	char* content = readFile(filename)

	//check length integrity
	ulng contentLen = strlen(content);
	if(contentLen%CPT__HC_INSTRUCTION_LENGTH != 0) {
		fprintf(
			stderr,
			"VisualLinux: Invalid number of bytes in HC file \"%s\" (multiples of %llu required, %llu found).\n",
			filename,
			CPT__HC_INSTRUCTION_LENGTH,
			contentLen
		);
		exit(EXIT_FAILURE);
	}

	//check instruction number
	ulng instructionsLen = contentLen / CPT__HC_INSTRUCTION_LENGTH;
	if(instructionsLen > instructionsLenLimit) {
		fprintf(
			stderr,
			"VisualLinux: Too much instructions taken from HC file \"%s\" (maximum %llu allowed, %llu found).\n",
			filename,
			instructionsLenLimit,
			instructionsLen
		);
		exit(EXIT_FAILURE);
	}

	//for each instruction to load
	instructions* instructions = malloc(instructionsLen * sizeof(instructions*));
	for(ulng i=0ULL; i < instructionsLen; i++) {

		//allocation + lineNbr
		instructions[i]          = malloc(sizeof(instruction));
		instructions[i]->lineNbr = lineNbrOffset + i;

		//

		//text field
		instructions[i]->text    = malloc(CPT__HC_INSTRUCTION_LENGTH * sizeof(char));
		instructions[i]->text[0] = content[];

		//
		instructions[i]-> = ;
		instructions[i]-> = ;
	}

	//return under instruction table
	return instructions;
}

//operate CPU
void operateCPU(cpt* computer) {
	instruction* currentInstruction = computer->instructions[computer->currentInstructionIndex];
	switch(currentInstruction->id){

		//print
		case CPT__ID_PR:
			puts("Performing HW operation PRINT with parameter.");
		break;

		//Move buffer read
		case CPT__ID_Mr:
			if(currentInstruction->pType == )short value = p
			puts("Performing HW operation MOVE BUFFER READ.");
		break;

		//Move buffer write
		case CPT__ID_Mw:
			puts("Performing HW operation MOVE BUFFER WRITE.");
		break;

		//add
		case CPT__ID_AD:
		break;

		//undefined instruction
		default:
			fprintf(
				stderr,
				"VisualLinux: Invalid HC instruction ID %i detected at line %llu : \"%s\".\n",
				(int)(currentLine->id),
				computer->currentProgramLine,
				currentLine->text
			);
			exit(EXIT_FAILURE);
		break;
	}
}
