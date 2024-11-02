#ifndef DISPLAY_TOOLS__H
#define DISPLAY_TOOLS__H






// ---------------- METRICS ----------------

//global
#define DT__WINDOW_WIDTH      720
#define DT__WINDOW_HEIGHT    1280
#define DT__GLOBAL_THICKNESS 1.1f

//text
#define DT__NUMBER_SIZE         0.08
#define DT__TEXT_SIZE           0.1
#define DT__TITLE_HEIGHT_OFFSET 20

//data boxes
#define DT__BYTE_BOX_HALFWIDTH          5
#define DT__BYTE_BOX_ASSUMED_TEXTWIDTH  18
#define DT__BYTE_BOX_ASSUMED_TEXTHEIGHT 10
#define DT__BYTE_BOX_HALFHEIGHT          6
#define DT__BYTE_BOX_WIDTH              (2*DT__BYTE_BOX_HALFWIDTH + DT__BYTE_BOX_ASSUMED_TEXTWIDTH)
#define DT__BYTE_BOX_HEIGHT             (2*DT__BYTE_BOX_HALFHEIGHT+ DT__BYTE_BOX_ASSUMED_TEXTHEIGHT)
#define DT__WORD_BOX_HALFWIDTH          5
#define DT__WORD_BOX_ASSUMED_TEXTWIDTH  35
#define DT__WORD_BOX_ASSUMED_TEXTHEIGHT 10
#define DT__WORD_BOX_HALFHEIGHT          6
#define DT__WORD_BOX_WIDTH              (2*DT__WORD_BOX_HALFWIDTH + DT__WORD_BOX_ASSUMED_TEXTWIDTH)
#define DT__WORD_BOX_HEIGHT             (2*DT__WORD_BOX_HALFHEIGHT+ DT__WORD_BOX_ASSUMED_TEXTHEIGHT)

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
#define DT__CPUMEM_SELECTION_SHIFT_X   3 //current selection
#define DT__CPUMEM_SELECTION_SHIFT_Y   5

//ram
#define DT__MAX_RAM_BYTE_PER_COLUMN 47
#define DT__RAM_PC_SHIFT_X           3 //program counter
#define DT__RAM_PC_SHIFT_Y           5
#define DT__RAM_SPC_SHIFT_X          5 //supervised program counter
#define DT__RAM_SPC_SHIFT_Y          7

//screen
#define DT__SCREEN_HALFWIDTH  200
#define DT__SCREEN_HALFHEIGHT 240
#define DT__SCREEN_SHIFT_X     10
#define DT__SCREEN_SHIFT_Y     16
#define DT__SCREEN_LINE_SHIFT  24

//current instruction
#define DT__CURRENT_INSTRUCTION_HALFWIDTH           20
#define DT__CURRENT_INSTRUCTION_ASSUMED_TEXTWIDTH  200
#define DT__CURRENT_INSTRUCTION_ASSUMED_TEXTHEIGHT  20
#define DT__CURRENT_INSTRUCTION_HALFHEIGHT          15
#define DT__CURRENT_INSTRUCTION_WIDTH              (2*DT__CURRENT_INSTRUCTION_HALFWIDTH  + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTWIDTH)
#define DT__CURRENT_INSTRUCTION_HEIGHT             (2*DT__CURRENT_INSTRUCTION_HALFHEIGHT + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTHEIGHT)
#define DT__CURRENT_INSTRUCTION_TEXT_SIZE          0.2

//global positionning
#define DT__INSTRUCTIONS_X          10
#define DT__INSTRUCTIONS_Y        1040
#define DT__CPUMEMS_X             1555
#define DT__CPUMEMS_Y             1035
#define DT__RAM_X                   10
#define DT__RAM_Y                 1035
#define DT__SCREEN_X              1710
#define DT__SCREEN_Y               260
#define DT__CURRENT_INSTRUCTION_X 1620
#define DT__CURRENT_INSTRUCTION_Y  525






// ---------------- COLORS ----------------

//global window
#define DT__COLOR_BACKGROUND_R 0
#define DT__COLOR_BACKGROUND_G 0
#define DT__COLOR_BACKGROUND_B 0

//box
#define DT__COLOR_BOX_TEXT_R 128
#define DT__COLOR_BOX_TEXT_G 128
#define DT__COLOR_BOX_TEXT_B 128

//titles
#define DT__COLOR_TITLE_TEXT_R   0
#define DT__COLOR_TITLE_TEXT_G 255
#define DT__COLOR_TITLE_TEXT_B   0

//ram program counter
#define DT__COLOR_RAM_PC_R   0
#define DT__COLOR_RAM_PC_G 255
#define DT__COLOR_RAM_PC_B   0

//ram supervised program counter
#define DT__COLOR_RAM_SPC_R 255
#define DT__COLOR_RAM_SPC_G   0
#define DT__COLOR_RAM_SPC_B   0

//cpumem selection
#define DT__COLOR_CPUMEM_SELECTION_R 110
#define DT__COLOR_CPUMEM_SELECTION_G 110
#define DT__COLOR_CPUMEM_SELECTION_B 255

//screen
#define DT__COLOR_SCREEN_BACKGROUND_R   0
#define DT__COLOR_SCREEN_BACKGROUND_G   0
#define DT__COLOR_SCREEN_BACKGROUND_B   0
#define DT__COLOR_SCREEN_STROKE_R 255
#define DT__COLOR_SCREEN_STROKE_G 255
#define DT__COLOR_SCREEN_STROKE_B   0
#define DT__COLOR_SCREEN_TEXT_R 255
#define DT__COLOR_SCREEN_TEXT_G 255
#define DT__COLOR_SCREEN_TEXT_B 255

//current instruction
#define DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_R  0
#define DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_G  0
#define DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_B 80
#define DT__COLOR_CURRENT_INSTRUCTION_STROKE_R   0
#define DT__COLOR_CURRENT_INSTRUCTION_STROKE_G 255
#define DT__COLOR_CURRENT_INSTRUCTION_STROKE_B   0
#define DT__COLOR_CURRENT_INSTRUCTION_TEXT_R   0
#define DT__COLOR_CURRENT_INSTRUCTION_TEXT_G 255
#define DT__COLOR_CURRENT_INSTRUCTION_TEXT_B   0






// ---------------- TOOLS ----------------

//generic
void drawByte(int x, int y, ubyt b);
void drawShort(int x, int y, ushr s);

//CPU
void displayCPUMems(int x, int y, cpt* c);
void displayRAM(int x, int y, cpt* c);

//virtual screen
void displayScreen(int x, int y, cpt* c);

//current instruction
void displaySupervisedInstruction(int x, int y, cpt* c);






#endif
