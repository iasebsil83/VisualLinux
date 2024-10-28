#ifndef COMPUTER__H
#define COMPUTER__H





// ---------------- DECLARATIONS ----------------

//should be default tools
#include "utils.h"

//HC instructions: to apply only on the first 2 bytes (instruction header)
#define CPT__INSTRUCTION_MODE_MASK     0x8000 //mode
#define CPT__INSTRUCTION_MODE_KERNEL   0x8000
#define CPT__INSTRUCTION_MODE_USER     0x0000
#define CPT__INSTRUCTION_PTYP_MASK     0x6000 //parameter type
#define CPT__INSTRUCTION_PTYP_MEMORY   0x4000
#define CPT__INSTRUCTION_PTYP_REGISTER 0x2000
#define CPT__INSTRUCTION_PTYP_VALUE    0x0000
#define CPT__INSTRUCTION_ID_MASK       0x1fff //ID
#define CPT__INSTRUCTION_ID_ADD        0x0000
#define CPT__INSTRUCTION_ID_MUL        0x0001
#define CPT__INSTRUCTION_ID_PRT        0x0002
#define CPT__INSTRUCTION_ID_JMP        0x0003
#define CPT__INSTRUCTION_ID_INP        0x0004
#define CPT__INSTRUCTION_ID_OUP        0x0005
#define CPT__INSTRUCTION_ID_REA        0x0006
#define CPT__INSTRUCTION_ID_WRI        0x0007
#define CPT__INSTRUCTION_ID_LOA        0x0008
#define CPT__INSTRUCTION_ID_ZER        0x0009
#define CPT__INSTRUCTION_ID_CHA        0x000a
#define CPT__INSTRUCTION_ID_GPC        0x000b
#define CPT__INSTRUCTION_ID_SUB        0x000c
#define CPT__INSTRUCTION_ID_DIV        0x000d
#define CPT__INSTRUCTION_ID_EQU        0x000e
#define CPT__INSTRUCTION_ID_NEQ        0x000f

//computer constants
#define CPT__CPUMEMS_LENGTH            8    //number of CPU mems available
#define CPT__CPUMEM_REGISTERS_LENGTH   8    //number of registers per CPU mem
#define CPT__CPUMEM_STACK_LENGTH       13   //number of stack slots per CPU mem
#define CPT__RAM_LENGTH                1544 //number of word available in RAM
#define CPT__SCREEN_LENGTH             20   //number of lines in screen

//cpu
typedef struct {
	ushr* registers;
	ushr* stack;
} cpuMem;

//computer
typedef struct {
	char*    storageDir;
	ushr     currentInstructionIndex;           //index of current instruction in RAM
	ushr     currentSupervisedInstructionIndex; //index of current SUPERVISED instruction in RAM
	cpuMem** cpuMems;                           //cpu
	ushr     currentCpuMemIndex;
	ushr*    ram;                               //ram
	char**   screen;                            //screen
} cpt;






// ---------------- FUNCTIONS ----------------

//tools
cpt* newComputer(char* storageDir);
void checkRAMAddress(ushr address);
ushr getRegisterValue(ushr* currentRegisters, ushr address);
char* readStringFromRAM(ushr* ram, ushr address);
void writeOnScreen(char** screen, char* line);

//execution
void loadFromStorage(ushr* ram, ulng startIndex, char* filename);
void stackPush(ushr* stack, ushr value);
ushr stackPop(ushr* stack);
void operateCPU(cpt* computer);
void loadKernel(cpt* computer);






#endif
