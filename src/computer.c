// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//own header
#include "computer.h"






// ---------------- TOOLS ----------------

//instantiate a new computer
cpt* newComputer(char* storageDir) {
	cpt* c = malloc(sizeof(cpt));

	//basic stuff
	c->storageDir       = storageDir;
	c->programCounter   = 0;
	c->supervisionIndex = 0;

	//cpu mems
	cpuMem** cpuMems = malloc(CPT__CPUMEMS_LENGTH * sizeof(cpuMem*));
	for(ushr i=0; i < CPT__CPUMEMS_LENGTH; i++) {
		cpuMems[i] = malloc(sizeof(cpuMem));
		cpuMem* cm = cpuMems[i];

		//registers
		ushr* registers = malloc(CPT__CPUMEM_REG_LENGTH * sizeof(ushr));
		for(ushr j=0; j < CPT__CPUMEM_REG_LENGTH; j++) { registers[j] = 0; }
		cm->registers = registers;

		//stack
		ushr* stack = malloc(CPT__CPUMEM_STACK_LENGTH * sizeof(ushr));
		for(ushr j=0; j < CPT__CPUMEM_STACK_LENGTH; j++) { stack[j] = 0; }
		cm->stack = stack;
	}
	c->cpuMems            = cpuMems;
	c->currentCpuMemIndex = 0;

	//ram
	ubyt* ram = malloc(CPT__RAM_LENGTH * sizeof(ubyt));
	for(ushr i=0; i < CPT__RAM_LENGTH; i++) { ram[i] = '\x00'; }
	c->ram = ram;

	//screen
	char** screen = malloc(CPT__SCREEN_LENGTH * sizeof(char*));
	for(ushr s=0ULL; s < CPT__SCREEN_LENGTH; s++) {
		screen[s]    = malloc(1 * sizeof(char)); //allocate empty string
		screen[s][0] = '\0';
	}
	c->screen = screen;

	//computer initialized
	return c;
}

//boundaries check
void checkRAMAddress(ushr address) {
	if(address >= CPT__RAM_LENGTH) {
		fprintf(stderr, "Invalid RAM address %i (maximum %i allowed).", address, CPT__RAM_LENGTH-1);
		exit(EXIT_FAILURE);
	}
}




// ---------------- CPU MEMS ----------------

//registers
ushr cpuMem__getRegisterValue(cpuMem* cm, ushr address) {
	if(address >= CPT__CPUMEM_REG_LENGTH) {
		fprintf(stderr, "Invalid register address %i (maximum %i allowed).", address, CPT__CPUMEM_REG_LENGTH-1);
		exit(EXIT_FAILURE);
	}

	//inside boundaries
	return cm->registers[address];
}

//stack
void cpuMem__stackPush(cpuMem* cm, ushr value) {
	ushr* stack = cm->stack;
	for(ushr i=CPT__CPUMEM_STACK_LENGTH-2; i >= 0; i--) { stack[i+1] = stack[i]; } //shift all-1 values
	stack[0] = value; //set new one
}

ushr cpuMem__stackPop(cpuMem* cm) {
	ushr* stack = cm->stack;
	ushr  toPop = stack[0];
	for(ushr i=0; i < CPT__CPUMEMS_LENGTH-1; i++) { stack[i] = stack[i+1]; } //shift all-1 values
	stack[CPT__CPUMEMS_LENGTH-1] = 0; //reset last value
	return toPop;
}




// ---------------- COMPUTER ----------------

//strings (taken from RAM)
char* cpt__readStringFromRAM(cpt* c, ushr address) {
	ubyt* ram = c->ram;
	checkRAMAddress(address);

	//read whole RAM from the first
	ushr length = 0;
	char current;
	for(ushr r=address; r < CPT__RAM_LENGTH; r++) {
		current = ram[r];

		//end of string reached
		if(current == '\0') { break; }

		//computing length
		length++;
	}

	//string creation
	char* result         = malloc(length+1);
	ushr  currentAddress = address;
	for(ushr r=0; r < length; r++) {
		result[r] = ram[currentAddress];
		currentAddress++;
	}
	result[length] = '\0';

	//return string
	return result;
}

//screen
void cpt__writeOnScreen(cpt* c, char* line) {
	char** screen = c->screen;

	//free first line
	free(screen[0]);

	//shift every lines
	ushr lastIndex = CPT__SCREEN_LENGTH-1;
	for(ushr s=0; s < lastIndex; s++) { screen[s] = screen[s+1]; }

	//write new line
	screen[lastIndex] = line;
}






// ---------------- EXECUTION ----------------

//operations
void cpt__loadFromStorage(cpt* c, ulng startIndex, char* filename) {
	buffer* content = readFile(filename);

	//program too big to be loaded
	if(startIndex + content->length > CPT__RAM_LENGTH) {
		fprintf(
			stderr,
			"Computer: Program too big to be loaded (%llu bytes to load starting from %llu, RAM length is %i).",
			content->length, startIndex, CPT__RAM_LENGTH
		);
		exit(EXIT_FAILURE);
	}

	//store in RAM starting from the correct position
	ubyt* ram   = c->ram + startIndex;
	char* input = content->data;
	for(ushr i=0; i < content->length; i++) { ram[i] = input[i]; }
	freeBuffer(content);
}

//extract a word from RAM
ushr cpt__getWordFromRAM(cpt* c, ushr address) {
	checkRAMAddress(address);
	ushr result = c->ram[address];
	result      = result << 8;
	result      = result | c->ram[address+1];
	return result;
}

//compute current instruction
void cpt__operateCPU(cpt* c) {
	cpuMem* currentCpuMem    = c->cpuMems[c->currentCpuMemIndex];
	ushr*   currentRegisters = currentCpuMem->registers;
	ubyt*   ram              = c->ram;

	//decompose current HC instruction
	ubyt currentByte   = ram[c->programCounter];
	ubyt id            =  currentByte & CPT__INSTRUCTION_MASK;
	ubyt registerIndex = (currentByte & CPT__REGINDEX_MASK) >> 5;

	//targetted register
	ushr* targettedRegisterAddress = currentRegisters+registerIndex;
	ushr  targettedRegisterValue   = currentRegisters[registerIndex];

	//redirections
	char* filename;
	ushr  one;
	switch(id){

		//nop (useless here)
		case CPT__INSTRUCTION_NOP:
			c->programCounter++;
		break;



		//memory management
		case CPT__INSTRUCTION_MOVE_MEM2REG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			targettedRegisterAddress[0] = cpt__getWordFromRAM(c, one);
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_MOVE_REG2MEM:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			ram[one  ] = targettedRegisterValue;
			ram[one+1] = targettedRegisterValue >> 8;
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_MOVE_REG2REG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for MOVE_REG2REG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_MOVE_VAL2REG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			targettedRegisterAddress[0] = one;
			c->programCounter += 3; //instruction + 1 word as parameter
		break;



		//cpuMems
		case CPT__INSTRUCTION_PUSH_REG:
			cpuMem__stackPush(currentCpuMem, targettedRegisterValue);
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_POP_REG:
			targettedRegisterAddress[0] = cpuMem__stackPop(currentCpuMem);
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_CPUMEM_SWITCH:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEMS_LENGTH) {
				fprintf(stderr, "Computer: Invalid cpuMem number %i given for CPUMEM_SWITCH instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			c->currentCpuMemIndex = one;
			c->programCounter += 3; //instruction + 1 word as parameter
		break;



		//control flow
		case CPT__INSTRUCTION_SKIPIFZ_REG:
			if(targettedRegisterValue == 0) { c->programCounter++; } //skip next byte in RAM (potentially next instruction)
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_PCSET_REG:
			targettedRegisterAddress[0] = c->programCounter;
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_PCGET_REG:
			c->programCounter = targettedRegisterValue;
			c->programCounter++;
		break;



		//operations
		case CPT__INSTRUCTION_ADD_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for ADD_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue + currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_MUL_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for MUL_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue * currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_LSHIFT_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for LSHIFT_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue << currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_RSHIFT_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for RSHIFT_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue >> currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_LOR_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for LOR_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue | currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_LAND_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for LAND_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue & currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;

		case CPT__INSTRUCTION_LXOR_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for LXOR_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			targettedRegisterAddress[0] = targettedRegisterValue ^ currentRegisters[one];
			c->programCounter += 3; //instruction + 1 word as parameter
		break;



		//misc
		case CPT__INSTRUCTION_PRINT_REG:
			cpt__writeOnScreen(c, cpt__readStringFromRAM(c, targettedRegisterValue));
			c->programCounter++;
		break;



		//external
		case CPT__INSTRUCTION_PLOAD_REGREG:
			one = cpt__getWordFromRAM(c, c->programCounter+1);
			if(one < CPT__CPUMEM_REG_LENGTH) {
				fprintf(stderr, "Computer: Invalid register number %i given for PLOAD_REGREG instruction (program counter at %i).", one, c->programCounter);
				exit(EXIT_FAILURE);
			}
			filename = cpt__readStringFromRAM(c, one);
			cpt__loadFromStorage(c, targettedRegisterValue, filename);
			free(filename);
			c->programCounter += 3; //instruction + 1 word as parameter
		break;



		//undefined yet
		case CPT__INSTRUCTION_UNDEFINEDB:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINEDA:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED9:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED8:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED7:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED6:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED5:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED4:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED3:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED2:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED1:
			c->programCounter++;
		break;

		case CPT__INSTRUCTION_UNDEFINED0:
			c->programCounter++;
		break;



		//invalid instruction (should never occur)
		default:
			fprintf(stderr, "Computer: [INTERNAL ERROR] Invalid HC instruction ID %i detected at RAM index %i.\n", id, c->programCounter);
			exit(EXIT_FAILURE);
		break;
	}

	//also stick supervision box to potentially new programCounter
	c->supervisionIndex = c->programCounter;
}

//kernel
void cpt__loadKernel(cpt* c) {

	//format kernel program filename
	ulng  kernelFilenameLen = strlen(c->storageDir) + 11ULL; //second item is strlen("/kernel.hc" + '\0')
	char* kernelFilename = malloc(kernelFilenameLen);
	sprintf(kernelFilename, "%s/kernel.hc", c->storageDir);

	//load content of kernel program
	cpt__loadFromStorage(c, 0, kernelFilename);
	free(kernelFilename);
}
