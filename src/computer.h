#ifndef COMPUTER__H
#define COMPUTER__H





// ---------------- DECLARATIONS ----------------

//should be default tools
#include "utils.h"

//HC instructions
#define CPT__INSTRUCTION_NOP           0x00 //nop (useless here)
#define CPT__INSTRUCTION_MOVE_MEM2REG  0x01 //memory
#define CPT__INSTRUCTION_MOVE_REG2MEM  0x02
#define CPT__INSTRUCTION_MOVE_REG2REG  0x03
#define CPT__INSTRUCTION_MOVE_VAL2REG  0x04
#define CPT__INSTRUCTION_PUSH_REG      0x05 //cpuMems
#define CPT__INSTRUCTION_POP_REG       0x06
#define CPT__INSTRUCTION_CPUMEM_SWITCH 0x07
#define CPT__INSTRUCTION_SKIPIFZ_REG   0x08 //control flow
#define CPT__INSTRUCTION_PCSET_REG     0x09
#define CPT__INSTRUCTION_PCGET_REG     0x0a
#define CPT__INSTRUCTION_ADD_REGREG    0x0b //arithmetic & logic
#define CPT__INSTRUCTION_MUL_REGREG    0x0c
#define CPT__INSTRUCTION_LSHIFT_REGREG 0x0d
#define CPT__INSTRUCTION_RSHIFT_REGREG 0x0e
#define CPT__INSTRUCTION_LOR_REGREG    0x0f
#define CPT__INSTRUCTION_LAND_REGREG   0x10
#define CPT__INSTRUCTION_LXOR_REGREG   0x11
#define CPT__INSTRUCTION_PRINT_REG     0x12 //misc
#define CPT__INSTRUCTION_PLOAD_REGREG  0x13 //external
#define CPT__INSTRUCTION_UNDEFINEDB    0x14
#define CPT__INSTRUCTION_UNDEFINEDA    0x15
#define CPT__INSTRUCTION_UNDEFINED9    0x16
#define CPT__INSTRUCTION_UNDEFINED8    0x17
#define CPT__INSTRUCTION_UNDEFINED7    0x18
#define CPT__INSTRUCTION_UNDEFINED6    0x19
#define CPT__INSTRUCTION_UNDEFINED5    0x1a
#define CPT__INSTRUCTION_UNDEFINED4    0x1b
#define CPT__INSTRUCTION_UNDEFINED3    0x1c
#define CPT__INSTRUCTION_UNDEFINED2    0x1d
#define CPT__INSTRUCTION_UNDEFINED1    0x1e
#define CPT__INSTRUCTION_UNDEFINED0    0x1f
#define CPT__INSTRUCTION_MASK          0x1f
#define CPT__REGINDEX_MASK             0x70

//computer constants
#define CPT__CPUMEMS_LENGTH      8    //number of CPU mems available
#define CPT__CPUMEM_REG_LENGTH   8    //number of registers per CPU mem
#define CPT__CPUMEM_STACK_LENGTH 13   //number of stack slots per CPU mem
#define CPT__RAM_LENGTH          2490 //number of bytes available in RAM
#define CPT__SCREEN_LENGTH       20   //number of lines in screen

//cpu
typedef struct {
	ushr* registers;
	ushr* stack;
} cpuMem;

//computer
typedef struct {
	char*    storageDir;
	ushr     programCounter;
	ushr     supervisionIndex; //user box for reading what's in RAM (not really part of the computer)
	cpuMem** cpuMems;            //cpu
	ushr     currentCpuMemIndex;
	ubyt*    ram;                //ram
	char**   screen;             //screen
} cpt;






// ---------------- FUNCTIONS ----------------

//tools
cpt*  newComputer(char* storageDir);
void  checkRAMAddress(ushr address);

//cpuMem
ushr cpuMem__getRegisterValue(cpuMem* cm, ushr address);
void cpuMem__stackPush(cpuMem* cm, ushr value);
ushr cpuMem__stackPop(cpuMem* cm);

//computer
char* cpt__readStringFromRAM(cpt* c, ushr address);
void  cpt__writeOnScreen(cpt* c, char* line);
void  cpt__loadFromStorage(cpt* c, ulng startIndex, char* filename);
ushr  cpt__getWordFromRAM(cpt* c, ushr address);
void  cpt__operateCPU(cpt* c);
void  cpt__loadKernel(cpt* c);






#endif
