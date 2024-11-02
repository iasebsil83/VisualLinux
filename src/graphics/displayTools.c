// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//graphics
#include "S2DE.h"

//computer
#include "../computer.h"

//own header
#include "displayTools.h"






// ---------------- DECLARATIONS ----------------

//S2DE variables
extern unsigned int S2DE_width;
extern unsigned int S2DE_height;






// ---------------- TOOLS ----------------

//generic
char halfByteToHex(ubyt hb) {
	if(hb >= 0  && hb <=  9) { return  hb       + '0'; }
	if(hb >= 10 && hb <= 15) { return (hb - 10) + 'a'; }
	return '#'; //invalid value (can also be an internal error)
}

//draw
void drawByte(int x, int y, ubyt b) {

	//get corresponding hex string
	char hexText[] = "XX";
	hexText[0] = halfByteToHex((b & 0xf0) >>  4);
	hexText[1] = halfByteToHex( b & 0x0f       );

	//draw text
	S2DE_setColor(DT__COLOR_BOX_TEXT_R, DT__COLOR_BOX_TEXT_G, DT__COLOR_BOX_TEXT_B);
	S2DE_rectangle(
		x - DT__BYTE_BOX_HALFWIDTH                                 , y-DT__BYTE_BOX_HALFHEIGHT,
		x + DT__BYTE_BOX_HALFWIDTH + DT__BYTE_BOX_ASSUMED_TEXTWIDTH, y+DT__BYTE_BOX_HALFHEIGHT + DT__BYTE_BOX_ASSUMED_TEXTHEIGHT,
		false
	);
	S2DE_text(hexText, DT__NUMBER_SIZE, x,y);
}

void drawShort(int x, int y, ushr s) {

	//get corresponding hex string
	char hexText[] = "XXXX";
	hexText[0] = halfByteToHex((s & 0xf000) >> 12);
	hexText[1] = halfByteToHex((s & 0x0f00) >>  8);
	hexText[2] = halfByteToHex((s & 0x00f0) >>  4);
	hexText[3] = halfByteToHex( s & 0x000f       );

	//draw text
	S2DE_setColor(DT__COLOR_BOX_TEXT_R, DT__COLOR_BOX_TEXT_G, DT__COLOR_BOX_TEXT_B);
	S2DE_rectangle(
		x - DT__WORD_BOX_HALFWIDTH                                 , y-DT__WORD_BOX_HALFHEIGHT,
		x + DT__WORD_BOX_HALFWIDTH + DT__WORD_BOX_ASSUMED_TEXTWIDTH, y+DT__WORD_BOX_HALFHEIGHT + DT__WORD_BOX_ASSUMED_TEXTHEIGHT,
		false
	);
	S2DE_text(hexText, DT__NUMBER_SIZE, x,y);
}

//CPU
void displayCPUMems(int x, int y, cpt* c) {
	int originalY         = y;
	char registerTitle[3] = "RX";

	//draw title
	S2DE_setColor(DT__COLOR_TITLE_TEXT_R, DT__COLOR_TITLE_TEXT_G, DT__COLOR_TITLE_TEXT_B);
	S2DE_text("CPU MEMs", DT__TEXT_SIZE, x, y + DT__WORD_BOX_HALFHEIGHT + DT__TITLE_HEIGHT_OFFSET);

	//draw registers name
	for(int r=0; r < CPT__CPUMEM_REG_LENGTH; r++) {
		registerTitle[1] = '0' + r;
		S2DE_text(registerTitle, DT__TEXT_SIZE, x - DT__WORD_BOX_HALFWIDTH - DT__CPUMEM_REGISTER_TITLE_OFFSET_WIDTH, y);

		//shift down for the next one
		y -= DT__WORD_BOX_HEIGHT;
	}

	//draw stack title
	S2DE_text(
		"stack", DT__TEXT_SIZE,
		x - DT__WORD_BOX_HALFWIDTH - DT__CPUMEM_STACK_TITLE_OFFSET_WIDTH,
		y - DT__CPUMEM_REGISTER_STACK_OFFSET_HEIGHT
	);

	//for each cpuMem
	for(int cm=0; cm < CPT__CPUMEMS_LENGTH; cm++) {
		y               = originalY;
		cpuMem* current = c->cpuMems[cm];

		//display registers
		for(int r=0; r < CPT__CPUMEM_REG_LENGTH; r++) {
			drawShort(x, y, current->registers[r]);

			//shift down for the next one
			y -= DT__WORD_BOX_HEIGHT;
		}

		//shift a bot to draw stack
		y -= DT__CPUMEM_REGISTER_STACK_OFFSET_HEIGHT;

		//display stack
		for(ulng s=0ULL; s < CPT__CPUMEM_STACK_LENGTH; s++) {
			drawShort(x,y, current->stack[s]);

			//shift down for the next one
			y -= DT__WORD_BOX_HEIGHT;
		}

		//current selection box
		if(cm == c->currentCpuMemIndex) {
			S2DE_setColor(DT__COLOR_CPUMEM_SELECTION_R, DT__COLOR_CPUMEM_SELECTION_G, DT__COLOR_CPUMEM_SELECTION_B);
			S2DE_rectangle(
				x - DT__WORD_BOX_HALFWIDTH                                  - DT__RAM_PC_SHIFT_X,         y+DT__WORD_BOX_HALFHEIGHT + DT__WORD_BOX_ASSUMED_TEXTHEIGHT - DT__CPUMEM_SELECTION_SHIFT_Y,
				x + DT__WORD_BOX_HALFWIDTH + DT__WORD_BOX_ASSUMED_TEXTWIDTH + DT__RAM_PC_SHIFT_X, originalY+DT__WORD_BOX_HALFHEIGHT + DT__WORD_BOX_ASSUMED_TEXTHEIGHT + DT__CPUMEM_SELECTION_SHIFT_Y,
				false
			);
		}

		//right shift to get to next cpuMem
		x += DT__WORD_BOX_WIDTH;
	}
}
void displayRAM(int x, int y, cpt* c) {

	//draw title
	S2DE_setColor(DT__COLOR_TITLE_TEXT_R, DT__COLOR_TITLE_TEXT_G, DT__COLOR_TITLE_TEXT_B);
	S2DE_text("RAM", DT__TEXT_SIZE, x, y + DT__BYTE_BOX_HALFHEIGHT + DT__TITLE_HEIGHT_OFFSET);

	//display memory blocks
	int originalY = y;
	for(ulng r=0ULL; r < CPT__RAM_LENGTH; r++) {

		//shift on a new column if necessary
		if(r != 0 && r%DT__MAX_RAM_BYTE_PER_COLUMN == 0) {
			x += DT__BYTE_BOX_WIDTH;
			y  = originalY;
		}

		//draw data box
		drawByte(x,y, c->ram[r]);

		//supervised program counter
		if(r == c->supervisionIndex) {
			S2DE_setColor(DT__COLOR_RAM_SPC_R, DT__COLOR_RAM_SPC_G, DT__COLOR_RAM_SPC_B);
			S2DE_rectangle(
				x - DT__BYTE_BOX_HALFWIDTH                                  - DT__RAM_SPC_SHIFT_X, y-DT__BYTE_BOX_HALFHEIGHT - DT__RAM_SPC_SHIFT_Y,
				x + DT__BYTE_BOX_HALFWIDTH + DT__BYTE_BOX_ASSUMED_TEXTWIDTH + DT__RAM_SPC_SHIFT_X, y+DT__BYTE_BOX_HALFHEIGHT + DT__BYTE_BOX_ASSUMED_TEXTHEIGHT + DT__RAM_SPC_SHIFT_Y,
				false
			);
		}

		//program counter
		if(r == c->programCounter) {
			S2DE_setColor(DT__COLOR_RAM_PC_R, DT__COLOR_RAM_PC_G, DT__COLOR_RAM_PC_B);
			S2DE_rectangle(
				x - DT__BYTE_BOX_HALFWIDTH                                  - DT__RAM_PC_SHIFT_X, y-DT__BYTE_BOX_HALFHEIGHT - DT__RAM_PC_SHIFT_Y,
				x + DT__BYTE_BOX_HALFWIDTH + DT__BYTE_BOX_ASSUMED_TEXTWIDTH + DT__RAM_PC_SHIFT_X, y+DT__BYTE_BOX_HALFHEIGHT + DT__BYTE_BOX_ASSUMED_TEXTHEIGHT + DT__RAM_PC_SHIFT_Y,
				false
			);
		}

		//shift down for the next one
		y -= DT__BYTE_BOX_HEIGHT;
	}
}

//virtual screen
void displayScreen(int x, int y, cpt* c) {
	char** screen = c->screen;

	//draw black screen
	S2DE_setColor(DT__COLOR_SCREEN_BACKGROUND_R, DT__COLOR_SCREEN_BACKGROUND_G, DT__COLOR_SCREEN_BACKGROUND_B);
	S2DE_rectangle(
		x-DT__SCREEN_HALFWIDTH, y-DT__SCREEN_HALFHEIGHT,
		x+DT__SCREEN_HALFWIDTH, y+DT__SCREEN_HALFHEIGHT,
		true
	);

	//draw yellow stroke
	S2DE_setColor(DT__COLOR_SCREEN_STROKE_R, DT__COLOR_SCREEN_STROKE_G, DT__COLOR_SCREEN_STROKE_B);
	S2DE_rectangle(
		x-DT__SCREEN_HALFWIDTH, y-DT__SCREEN_HALFHEIGHT,
		x+DT__SCREEN_HALFWIDTH, y+DT__SCREEN_HALFHEIGHT,
		false
	);

	//draw lines
	S2DE_setColor(DT__COLOR_SCREEN_TEXT_B, DT__COLOR_SCREEN_TEXT_B, DT__COLOR_SCREEN_TEXT_B);
	for(ulng s=0ULL; s < CPT__SCREEN_LENGTH; s++) {
		S2DE_text(
			screen[s],    DT__TEXT_SIZE,
			x-DT__SCREEN_HALFWIDTH + DT__SCREEN_SHIFT_X, y+DT__SCREEN_HALFHEIGHT - DT__SCREEN_SHIFT_Y - (s*DT__SCREEN_LINE_SHIFT)
		);
	}
}

void displaySupervisedInstruction(int x, int y, cpt* c) {
	ubyt* ram   = c->ram;
	char text[] = "#### ####";

	//ignore special case (invalid instruction)
	if(c->supervisionIndex != CPT__RAM_LENGTH-1ULL) {

		//decompose current supervised instruction
		ubyt currentByte   = ram[c->supervisionIndex];
		ubyt id            =  currentByte & CPT__INSTRUCTION_MASK;
		ubyt registerIndex = (currentByte & CPT__REGINDEX_MASK) >> 5;

		//instruction ID
		switch(id) {
			case CPT__INSTRUCTION_NOP:           text[0] = 'N'; text[1] = 'O'; text[2] = 'P'; break;
			case CPT__INSTRUCTION_MOVE_MEM2REG:  text[0] = 'M'; text[1] = 'M'; text[2] = 'R'; break;
			case CPT__INSTRUCTION_MOVE_REG2MEM:  text[0] = 'M'; text[1] = 'R'; text[2] = 'M'; break;
			case CPT__INSTRUCTION_MOVE_REG2REG:  text[0] = 'M'; text[1] = 'R'; text[2] = 'R'; break;
			case CPT__INSTRUCTION_MOVE_VAL2REG:  text[0] = 'M'; text[1] = 'V'; text[2] = 'R'; break;
			case CPT__INSTRUCTION_PUSH_REG:      text[0] = 'P'; text[1] = 'U'; text[2] = 'S'; break;
			case CPT__INSTRUCTION_POP_REG:       text[0] = 'P'; text[1] = 'O'; text[2] = 'P'; break;
			case CPT__INSTRUCTION_CPUMEM_SWITCH: text[0] = 'S'; text[1] = 'W'; text[2] = 'I'; break;
			case CPT__INSTRUCTION_SKIPIFZ_REG:   text[0] = 'S'; text[1] = 'K'; text[2] = 'F'; break;
			case CPT__INSTRUCTION_PCSET_REG:     text[0] = 'P'; text[1] = 'C'; text[2] = 'S'; break;
			case CPT__INSTRUCTION_PCGET_REG:     text[0] = 'P'; text[1] = 'C'; text[2] = 'G'; break;
			case CPT__INSTRUCTION_ADD_REGREG:    text[0] = 'A'; text[1] = 'D'; text[2] = 'D'; break;
			case CPT__INSTRUCTION_MUL_REGREG:    text[0] = 'M'; text[1] = 'U'; text[2] = 'L'; break;
			case CPT__INSTRUCTION_LSHIFT_REGREG: text[0] = 'L'; text[1] = 'S'; text[2] = 'H'; break;
			case CPT__INSTRUCTION_RSHIFT_REGREG: text[0] = 'R'; text[1] = 'S'; text[2] = 'H'; break;
			case CPT__INSTRUCTION_LOR_REGREG:    text[0] = 'L'; text[1] = 'O'; text[2] = 'R'; break;
			case CPT__INSTRUCTION_LAND_REGREG:   text[0] = 'L'; text[1] = 'A'; text[2] = 'N'; break;
			case CPT__INSTRUCTION_LXOR_REGREG:   text[0] = 'L'; text[1] = 'X'; text[2] = 'O'; break;
			case CPT__INSTRUCTION_PRINT_REG:     text[0] = 'P'; text[1] = 'R'; text[2] = 'T'; break;
			case CPT__INSTRUCTION_PLOAD_REGREG:  text[0] = 'L'; text[1] = 'O'; text[2] = 'A'; break;
			case CPT__INSTRUCTION_UNDEFINEDB:    text[0] = 'U'; text[1] = 'N'; text[2] = 'B'; break;
			case CPT__INSTRUCTION_UNDEFINEDA:    text[0] = 'U'; text[1] = 'N'; text[2] = 'A'; break;
			case CPT__INSTRUCTION_UNDEFINED9:    text[0] = 'U'; text[1] = 'N'; text[2] = '9'; break;
			case CPT__INSTRUCTION_UNDEFINED8:    text[0] = 'U'; text[1] = 'N'; text[2] = '8'; break;
			case CPT__INSTRUCTION_UNDEFINED7:    text[0] = 'U'; text[1] = 'N'; text[2] = '7'; break;
			case CPT__INSTRUCTION_UNDEFINED6:    text[0] = 'U'; text[1] = 'N'; text[2] = '6'; break;
			case CPT__INSTRUCTION_UNDEFINED5:    text[0] = 'U'; text[1] = 'N'; text[2] = '5'; break;
			case CPT__INSTRUCTION_UNDEFINED4:    text[0] = 'U'; text[1] = 'N'; text[2] = '4'; break;
			case CPT__INSTRUCTION_UNDEFINED3:    text[0] = 'U'; text[1] = 'N'; text[2] = '3'; break;
			case CPT__INSTRUCTION_UNDEFINED2:    text[0] = 'U'; text[1] = 'N'; text[2] = '2'; break;
			case CPT__INSTRUCTION_UNDEFINED1:    text[0] = 'U'; text[1] = 'N'; text[2] = '1'; break;
			case CPT__INSTRUCTION_UNDEFINED0:    text[0] = 'U'; text[1] = 'N'; text[2] = '0'; break;
		}

		//targetted register
		text[3] = registerIndex + '0';

		//following value
		if(c->supervisionIndex != CPT__RAM_LENGTH-1) {
			ushr nextWord = cpt__getWordFromRAM(c, c->supervisionIndex+1);
			text[5] = halfByteToHex((nextWord & 0xf000) >> 12);
			text[6] = halfByteToHex((nextWord & 0x0f00) >>  8);
			text[7] = halfByteToHex((nextWord & 0x00f0) >>  4);
			text[8] = halfByteToHex( nextWord & 0x000f       );
		}
	}

	//draw box background
	S2DE_setColor(
		DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_R,
		DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_G,
		DT__COLOR_CURRENT_INSTRUCTION_BACKGROUND_B
	);
	S2DE_rectangle(
		x - DT__CURRENT_INSTRUCTION_HALFWIDTH                                            , y-DT__CURRENT_INSTRUCTION_HALFHEIGHT,
		x + DT__CURRENT_INSTRUCTION_HALFWIDTH + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTWIDTH, y+DT__CURRENT_INSTRUCTION_HALFHEIGHT + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTHEIGHT,
		true
	);

	//draw box stroke
	S2DE_setColor(
		DT__COLOR_CURRENT_INSTRUCTION_STROKE_R,
		DT__COLOR_CURRENT_INSTRUCTION_STROKE_G,
		DT__COLOR_CURRENT_INSTRUCTION_STROKE_B
	);
	S2DE_rectangle(
		x - DT__CURRENT_INSTRUCTION_HALFWIDTH                                            , y-DT__CURRENT_INSTRUCTION_HALFHEIGHT,
		x + DT__CURRENT_INSTRUCTION_HALFWIDTH + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTWIDTH, y+DT__CURRENT_INSTRUCTION_HALFHEIGHT + DT__CURRENT_INSTRUCTION_ASSUMED_TEXTHEIGHT,
		false
	);

	//draw text
	S2DE_setColor(
		DT__COLOR_CURRENT_INSTRUCTION_TEXT_R,
		DT__COLOR_CURRENT_INSTRUCTION_TEXT_G,
		DT__COLOR_CURRENT_INSTRUCTION_TEXT_B
	);
	S2DE_text(text, DT__CURRENT_INSTRUCTION_TEXT_SIZE, x,y);
}
