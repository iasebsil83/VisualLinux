#ifndef DISPLAY_TOOLS__H
#define DISPLAY_TOOLS__H






// ---------------- METRICS ----------------

//text
#define DT__NUMBER_SIZE         0.08
#define DT__TEXT_SIZE           0.1
#define DT__TITLE_HEIGHT_OFFSET 10

//data boxes
#define DT__DATA_BOX_HALFWIDTH         5
#define DT__DATA_BOX_ASSUMED_TEXTWIDTH 35
#define DT__DATA_BOX_HALFHEIGHT        11
#define DT__DATA_BOX_WIDTH             (2*DT__DATA_BOX_HALFWIDTH + DT__DATA_BOX_ASSUMED_TEXTWIDTH)
#define DT__DATA_BOX_HEIGHT            (2*DT__DATA_BOX_HALFHEIGHT)

//HC instructions
#define DT__MAX_INSTRUCTIONS_PER_COLUMN       31
#define DT__INSTRUCTION_BOX_HALFWIDTH          5
#define DT__INSTRUCTION_BOX_ASSUMED_TEXTWIDTH 80
#define DT__INSTRUCTION_BOX_HALFHEIGHT        11
#define DT__INSTRUCTION_BOX_WIDTH             (2*DT__INSTRUCTION_BOX_HALFWIDTH + DT__INSTRUCTION_BOX_ASSUMED_TEXTWIDTH)
#define DT__INSTRUCTION_BOX_HEIGHT            (2*DT__INSTRUCTION_BOX_HALFHEIGHT)

//cpumem
#define DT__CPUMEM_REGISTER_TITLE_OFFSET_WIDTH  25
#define DT__CPUMEM_STACK_TITLE_OFFSET_WIDTH     55
#define DT__CPUMEM_REGISTER_STACK_OFFSET_HEIGHT 20

//ram
#define DT__MAX_RAM_WORD_PER_COLUMN 15

//screen
#define DT__SCREEN_HALFWIDTH  200
#define DT__SCREEN_HALFHEIGHT 110

//global positionning
#define DT__INSTRUCTIONS_X   10
#define DT__INSTRUCTIONS_Y 1040
#define DT__CPUMEMS_X      1555
#define DT__CPUMEMS_Y      1040
#define DT__RAM_X            10
#define DT__RAM_Y           320
#define DT__SCREEN_X       1710
#define DT__SCREEN_Y        450






// ---------------- TOOLS ----------------

//generic
void drawShort(int x, int y, unsigned short s);

//program lines
void displayInstructions(int x, int y, cpt* computer);

//CPU
void displayCPUMems(int x, int y, cpt* computer);
void displayRAM(int x, int y, cpt* computer);

//virtual screen
void displayScreen(int x, int y, char* text);






#endif
