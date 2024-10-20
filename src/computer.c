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
	c->storageDir                        = storageDir;
	c->currentInstructionIndex           = 0;
	c->currentSupervisedInstructionIndex = 0;

	//cpu mems
	cpuMem** cpuMems = malloc(CPT__CPUMEMS_LENGTH * sizeof(cpuMem*));
	for(ushr i=0; i < CPT__CPUMEMS_LENGTH; i++) {
		cpuMems[i] = malloc(sizeof(cpuMem));
		cpuMem* cm = cpuMems[i];

		//registers
		ushr* registers = malloc(CPT__CPUMEM_REGISTERS_LENGTH * sizeof(ushr));
		for(ushr j=0; j < CPT__CPUMEM_REGISTERS_LENGTH; j++) { registers[j] = 0; }
		cm->registers = registers;

		//stack
		ushr* stack = malloc(CPT__CPUMEM_STACK_LENGTH * sizeof(ushr));
		for(ushr j=0; j < CPT__CPUMEM_STACK_LENGTH; j++) { stack[j] = 0; }
		cm->stack = stack;
	}
	c->cpuMems            = cpuMems;
	c->currentCpuMemIndex = 0ULL;

	//ram
	ushr* ram = malloc(CPT__RAM_LENGTH * sizeof(ushr));
	for(ushr i=0; i < CPT__RAM_LENGTH; i++) { ram[i] = 0; }
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

//boudaries check
void checkRAMAddress(ushr address) {
	if(address >= CPT__RAM_LENGTH) {
		fprintf(stderr, "Invalid RAM address %i (maximum %i allowed).", address, CPT__RAM_LENGTH-1);
		exit(EXIT_FAILURE);
	}
}
ushr getRegisterValue(ushr* currentRegisters, ushr address) {
	if(address >= CPT__CPUMEM_REGISTERS_LENGTH) {
		fprintf(stderr, "Invalid register address %i (maximum %i allowed).", address, CPT__CPUMEM_REGISTERS_LENGTH-1);
		exit(EXIT_FAILURE);
	}

	//inside boundaries
	return currentRegisters[address];
}

//ram strings
char* readStringFromRAM(ushr* ram, ushr address) {
	checkRAMAddress(address);

	//read whole RAM from the first
	ushr currentValue;
	char c1, c2;
	ushr length = 0;
	for(ushr r=address; r < CPT__RAM_LENGTH; r++) {
		currentValue = ram[r];

		//read 2 by 2
		c1 = (currentValue & 0xff00) >> 8;
		c2 =  currentValue & 0x00ff;

		//end of string reached
		if(c1 == '\0') {           break; }
		if(c2 == '\0') { length++; break; }

		//not terminated => increase length
		length += 2;
	}

	//string creation
	char* result   = malloc(length+1);
	ushr  ramIndex = address;
	for(ushr s=0; s < length; s++) {
		currentValue = ram[ramIndex];

		//storing firt byte (and not touching RAM index)
		if(s%2 == 0) { result[s] = (currentValue & 0xff00) >> 8; }

		//storing second byte (and increasing RAM index)
		else { result[s] = currentValue & 0x00ff; ramIndex++; }
	}
	result[length] = '\0';

	//return string
	return result;
}

//screen
void writeOnScreen(char** screen, char* line) {

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
void loadFromStorage(ushr* ram, ulng startIndex, char* filename) {
	buffer* content = readFile(filename);
	ulng    wLength = 2ULL * content->length;

	//program too big to be loaded
	if(startIndex + wLength >= CPT__RAM_LENGTH) {
		fprintf(
			stderr,
			"Computer: Program too big to be loaded (%llu words to load starting from %llu, maximum RAM length is %i).",
			wLength, startIndex, CPT__RAM_LENGTH
		);
		exit(EXIT_FAILURE);
	}

	//store in RAM as we had a byte array (starting from the correct position of course)
	char* bRam  = (char*)(ram + startIndex);
	char* input = content->data;
	for(ulng i=0; i < content->length; i++) { bRam[i] = input[i]; }
	freeBuffer(content);
}
void stackPush(ushr* stack, ushr value) {
	for(ushr i=CPT__CPUMEM_STACK_LENGTH-2; i >= 0; i--) { stack[i+1] = stack[i]; } //shift all-1 values
	stack[0] = value; //set new one
}
ushr stackPop(ushr* stack) {
	ushr toPop = stack[0];
	for(ushr i=0; i < CPT__CPUMEMS_LENGTH-1; i++) { stack[i] = stack[i+1]; } //shift all-1 values
	stack[CPT__CPUMEMS_LENGTH-1] = 0; //reset last value
	return toPop;
}

//operate CPU
void operateCPU(cpt* computer) {
	cpuMem* currentCpuMem    = computer->cpuMems[computer->currentCpuMemIndex];
	ushr*   currentRegisters = currentCpuMem->registers;
	ushr*   currentStack     = currentCpuMem->stack;
	ushr*   ram              = computer->ram;

	//decompose current HC instruction
	ushr header = ram[computer->currentInstructionIndex];
	ushr mode   = header & CPT__INSTRUCTION_MODE_MASK;
	ushr pType  = header & CPT__INSTRUCTION_PTYP_MASK;
	ushr id     = header & CPT__INSTRUCTION_ID_MASK;
	ushr rawValue = ram[computer->currentInstructionIndex+1];
	ushr registerValue = 0;
	if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { registerValue = getRegisterValue(currentRegisters, rawValue); }

	//redirections
	char* filename;
	ushr one;
	ushr r0 = currentRegisters[0];
	switch(id){

		//add
		case CPT__INSTRUCTION_ID_ADD:
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            one = registerValue; }
			else                                        { checkRAMAddress(rawValue); one = ram[rawValue]; }
			currentRegisters[0] = r0 + one;
		break;

		//multiply
		case CPT__INSTRUCTION_ID_MUL:
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            one = registerValue; }
			else                                        { checkRAMAddress(rawValue); one = ram[rawValue]; }
			currentRegisters[0] = r0 * one;
		break;

		//print
		case CPT__INSTRUCTION_ID_PRT:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			writeOnScreen(computer->screen, readStringFromRAM(ram, one));
		break;

		//jump
		case CPT__INSTRUCTION_ID_JMP:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			computer->currentInstructionIndex += one/2 - 2; //shift -2 to take into account the auto-increment
		break;

		//input to stack
		case CPT__INSTRUCTION_ID_INP:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			stackPush(currentStack, one);
		break;

		//output from stack
		case CPT__INSTRUCTION_ID_OUP:
			one = stackPop(currentStack);
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            currentRegisters[rawValue] = one; }
			else                                        { checkRAMAddress(rawValue);              ram[rawValue] = one; }
		break;

		//write from Memory
		case CPT__INSTRUCTION_ID_MEM:
			one = ram[r0];
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            currentRegisters[rawValue] = one; }
			else                                        { checkRAMAddress(rawValue);              ram[rawValue] = one; }
		break;

		//write into Register
		case CPT__INSTRUCTION_ID_REG:
			one = getRegisterValue(currentRegisters, r0);
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            currentRegisters[rawValue] = one; }
			else                                        { checkRAMAddress(rawValue);              ram[rawValue] = one; }
		break;

		//load
		case CPT__INSTRUCTION_ID_LOA:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			filename = readStringFromRAM(ram, r0);
			loadFromStorage(ram, one, filename);
			free(filename);
		break;

		//zero
		case CPT__INSTRUCTION_ID_ZER:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			for(ushr i=0; i < one; i++) { ram[r0+i] = 0; }
		break;

		//change CPU mem
		case CPT__INSTRUCTION_ID_CHA:
			one = rawValue;
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) { one = registerValue; }
			if(one > CPT__CPUMEMS_LENGTH) {
				fprintf(stderr, "Computer: Invalid CPUMEM index given %i (maximum allowed is %i).", one, CPT__CPUMEMS_LENGTH-1);
				exit(EXIT_FAILURE);
			}
			computer->currentCpuMemIndex = one;
		break;

		//get Program Counter
		case CPT__INSTRUCTION_ID_GPC:
			if(pType == CPT__INSTRUCTION_PTYP_REGISTER) {                            currentRegisters[rawValue] = computer->currentInstructionIndex; }
			else                                        { checkRAMAddress(rawValue);              ram[rawValue] = computer->currentInstructionIndex; }
		break;

		//undefined instruction
		default:
			fprintf(stderr, "Computer: Invalid HC instruction ID %i detected at ram index %i.\n", id, computer->currentInstructionIndex);
			exit(EXIT_FAILURE);
		break;
	}

	//increase instruction index
	computer->currentInstructionIndex += 2; //2 words per instruction (4 bytes steps)

	//also set supervised instruction back to current execution
	computer->currentSupervisedInstructionIndex = computer->currentInstructionIndex;
}

//kernel
void loadKernel(cpt* computer) {

	//format kernel program filename
	ulng  kernelFilenameLen = strlen(computer->storageDir) + 11ULL; //second item is strlen("/kernel.hc" + '\0')
	char* kernelFilename = malloc(kernelFilenameLen);
	sprintf(kernelFilename, "%s/kernel.hc", computer->storageDir);

	//load content of kernel program
	loadFromStorage(computer->ram, 0, kernelFilename);
	free(kernelFilename);
}
