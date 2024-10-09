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
	char  name;
	char  pType;   //parsed: parameter
	short pValue;
} instruction;

//HC constants
#define CPT__SINGLE_INSTRUCTION_LENGTH 8ULL   //length of a single HC instruction text (including terminating line feed) : MNTVVVVl
#define CPT__INSTRUCTIONS_LENGTH       341ULL //maximum number of instructions loaded
#define CPT__CPUMEMS_LENGTH            8ULL   //number of CPU mems available
#define CPT__CPUMEM_REGISTERS_LENGTH   8ULL   //number of registers per CPU mem
#define CPT__CPUMEM_STACK_LENGTH       13ULL  //number of stack slots per CPU mem
#define CPT__RAM_LENGTH                630ULL //number of word available in RAM

//cpu
typedef struct {
	short* registers;
	short* stack;
} cpuMem;

//computer
typedef struct {
	char*         storageDir;
	instruction** instructions;            //program
	ulng          currentInstructionIndex;
	cpuMem**      cpuMems;                 //cpu
	ulng          currentCpuMemIndex;
	short*        ram;                     //ram
} cpt;






// ---------------- FUNCTIONS ----------------

//tools
char* readFile(char* filename);
cpt* newComputer(char* storageDir);

//execution
void loadHCContent(char* content, instruction** instructions, ulng startingIndex);
void operateCPU(cpt* computer);
void loadKernel(cpt* computer);






#endif
