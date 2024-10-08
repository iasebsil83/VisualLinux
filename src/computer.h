#ifndef COMPUTER__H
#define COMPUTER__H





// ---------------- DECLARATIONS ----------------

//useful
typedef unsigned long long ulng;

//instructions
typedef struct {
	ulng  lineNbr; //visual information
	char* text;
	char  mode;    //parsed: name
	char  id;
	char  pType;   //parsed: parameter
	short pValue;
} instruction;

//HC constants
#define CPT__HC_INSTRUCTION_LENGTH 9ULL    //including terminating line feed : mIITVVVVl
#define CPT__HC_MAX_INSTRUCTIONS   1024ULL //maximum number of instructions that can be stored
#define CPT__TYPE_VALUE    '\x00'
#define CPT__TYPE_REGISTER '\x01'
#define CPT__ID_PR '\x00'
#define CPT__ID_Mr '\x01'
#define CPT__ID_Mw '\x02'
#define CPT__ID_AD '\x03'

//cpu
typedef struct {
	ulng* registers;
	ulng* stack;
} cpuMem;

//computer
typedef struct {
	instruction* instructions;            //program
	ulng         currentInstructionIndex;
	cpuMem*      cpuMems;                 //cpu
	ulng         currentCpuMemIndex;
	ulng*        ram;                     //ram
} cpt;





#endif
