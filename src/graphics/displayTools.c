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
char halfByteToHex(char hb) {
	if(hb >= 0  && hb <=  9) { return  hb       + '0'; }
	if(hb >= 10 && hb <= 15) { return (hb - 10) + 'a'; }
	return '#'; //invalid value (can also be an internal error)
}
void drawShort(int x, int y, unsigned short s) {

	//get corresponding hex string
	char hexText[] = "XXXX";
	hexText[0] = halfByteToHex((s & 0xf000) >> 12);
	hexText[1] = halfByteToHex((s & 0x0f00) >>  8);
	hexText[2] = halfByteToHex((s & 0x00f0) >>  4);
	hexText[3] = halfByteToHex( s & 0x000f       );

	//draw text
	S2DE_setColor(DT__COLOR_BOX_TEXT_R, DT__COLOR_BOX_TEXT_G, DT__COLOR_BOX_TEXT_B);
	S2DE_rectangle(
		x - DT__DATA_BOX_HALFWIDTH                                 , y-DT__DATA_BOX_HALFHEIGHT,
		x + DT__DATA_BOX_HALFWIDTH + DT__DATA_BOX_ASSUMED_TEXTWIDTH, y+DT__DATA_BOX_HALFHEIGHT,
		false
	);
	S2DE_text(hexText, DT__NUMBER_SIZE, x,y);
}

//CPU
void displayCPUMems(int x, int y, cpt* computer) {
	int originalY         = y;
	char registerTitle[3] = "RX";

	//draw title
	S2DE_setColor(DT__COLOR_TITLE_TEXT_R, DT__COLOR_TITLE_TEXT_G, DT__COLOR_TITLE_TEXT_B);
	S2DE_text("CPU MEMs", DT__TEXT_SIZE, x, y + DT__DATA_BOX_HALFHEIGHT + DT__TITLE_HEIGHT_OFFSET);

	//draw registers name
	for(int r=0; r < CPT__CPUMEM_REGISTERS_LENGTH; r++) {
		registerTitle[1] = '0' + r;
		S2DE_text(registerTitle, DT__TEXT_SIZE, x - DT__DATA_BOX_HALFWIDTH - DT__CPUMEM_REGISTER_TITLE_OFFSET_WIDTH, y);

		//shift down for the next one
		y -= DT__DATA_BOX_HEIGHT;
	}

	//draw stack title
	S2DE_text(
		"stack", DT__TEXT_SIZE,
		x - DT__DATA_BOX_HALFWIDTH - DT__CPUMEM_STACK_TITLE_OFFSET_WIDTH,
		y - DT__CPUMEM_REGISTER_STACK_OFFSET_HEIGHT
	);

	//for each cpuMem
	for(int c=0; c < CPT__CPUMEMS_LENGTH; c++) {
		y               = originalY;
		cpuMem* current = computer->cpuMems[c];

		//display registers
		for(int r=0; r < CPT__CPUMEM_REGISTERS_LENGTH; r++) {
			drawShort(x, y, current->registers[r]);

			//shift down for the next one
			y -= DT__DATA_BOX_HEIGHT;
		}

		//shift a bot to draw stack
		y -= DT__CPUMEM_REGISTER_STACK_OFFSET_HEIGHT;

		//display stack
		for(ulng s=0ULL; s < CPT__CPUMEM_STACK_LENGTH; s++) {
			drawShort(x,y, current->stack[s]);

			//shift down for the next one
			y -= DT__DATA_BOX_HEIGHT;
		}

		//right shift to get to next cpuMem
		x += DT__DATA_BOX_WIDTH;
	}
}
void displayRAM(int x, int y, cpt* computer) {

	//draw title
	S2DE_setColor(DT__COLOR_TITLE_TEXT_R, DT__COLOR_TITLE_TEXT_G, DT__COLOR_TITLE_TEXT_B);
	S2DE_text("RAM", DT__TEXT_SIZE, x, y + DT__DATA_BOX_HALFHEIGHT + DT__TITLE_HEIGHT_OFFSET);

	//display memory blocks
	int originalY = y;
	for(ulng r=0ULL; r < CPT__RAM_LENGTH; r++) {

		//shift on a new column if necessary
		if(r != 0 && r%DT__MAX_RAM_WORD_PER_COLUMN == 0) {
			x += DT__DATA_BOX_WIDTH;
			y  = originalY;
		}

		//draw data box
		drawShort(x,y, computer->ram[r]);

		//program counter
		if(r == computer->currentInstructionIndex) {
			S2DE_setColor(DT__COLOR_RAM_PC_R, DT__COLOR_RAM_PC_G, DT__COLOR_RAM_PC_B);
			S2DE_rectangle(
				x - DT__DATA_BOX_HALFWIDTH                                  - DT__RAM_PC_SHIFT_X, y-DT__DATA_BOX_HALFHEIGHT - DT__RAM_PC_SHIFT_Y,
				x + DT__DATA_BOX_HALFWIDTH + DT__DATA_BOX_ASSUMED_TEXTWIDTH + DT__RAM_PC_SHIFT_X, y+DT__DATA_BOX_HALFHEIGHT + DT__RAM_PC_SHIFT_Y,
				false
			);
		}

		//shift down for the next one
		y -= DT__DATA_BOX_HEIGHT;
	}
}

//virtual screen
void displayScreen(int x, int y, cpt* computer) {

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
			computer->screen[s],    DT__TEXT_SIZE,
			x-DT__SCREEN_HALFWIDTH + DT__SCREEN_SHIFT_X, y+DT__SCREEN_HALFHEIGHT - DT__SCREEN_SHIFT_Y - (s*DT__SCREEN_LINE_SHIFT)
		);
	}
}

void displayCurrentInstruction(int x, int y, cpt* computer) {
	char text[] = "#########";

	//ignore special case (invalid instruction)
	if(computer->currentInstructionIndex != CPT__RAM_LENGTH-1ULL) {

		//decompose current instruction
		ushr header = computer->ram[computer->currentInstructionIndex  ];
		ushr value  = computer->ram[computer->currentInstructionIndex+1];
		ushr mode   = header & CPT__INSTRUCTION_MODE_MASK;
		ushr ptype  = header & CPT__INSTRUCTION_PTYP_MASK;
		ushr id     = header & CPT__INSTRUCTION_ID_MASK;

		//mode
		switch(mode) {
			case CPT__INSTRUCTION_MODE_KERNEL: text[0] = 'k'; break;
			case CPT__INSTRUCTION_MODE_USER:   text[0] = 'u'; break;
		}

		//parameter type
		switch(ptype) {
			case CPT__INSTRUCTION_PTYP_REGISTER: text[4] = 'r'; break;
			case CPT__INSTRUCTION_PTYP_VALUE:    text[4] = 'v'; break;
		}

		//instruction ID
		switch(id) {
			case CPT__INSTRUCTION_ID_ADD: text[1] = 'A'; text[2] = 'D'; text[3] = 'D'; break;
			case CPT__INSTRUCTION_ID_MUL: text[1] = 'M'; text[2] = 'U'; text[3] = 'L'; break;
			case CPT__INSTRUCTION_ID_PRT: text[1] = 'P'; text[2] = 'R'; text[3] = 'T'; break;
			case CPT__INSTRUCTION_ID_JMP: text[1] = 'J'; text[2] = 'M'; text[3] = 'P'; break;
			case CPT__INSTRUCTION_ID_INP: text[1] = 'I'; text[2] = 'N'; text[3] = 'P'; break;
			case CPT__INSTRUCTION_ID_OUP: text[1] = 'O'; text[2] = 'U'; text[3] = 'P'; break;
			case CPT__INSTRUCTION_ID_MEM: text[1] = 'M'; text[2] = 'E'; text[3] = 'M'; break;
			case CPT__INSTRUCTION_ID_REG: text[1] = 'R'; text[2] = 'E'; text[3] = 'G'; break;
			case CPT__INSTRUCTION_ID_LOA: text[1] = 'L'; text[2] = 'O'; text[3] = 'A'; break;
			case CPT__INSTRUCTION_ID_ZER: text[1] = 'Z'; text[2] = 'E'; text[3] = 'R'; break;
			case CPT__INSTRUCTION_ID_CHA: text[1] = 'C'; text[2] = 'H'; text[3] = 'A'; break;
			case CPT__INSTRUCTION_ID_GPC: text[1] = 'G'; text[2] = 'P'; text[3] = 'C'; break;
		}

		//value
		text[5] = halfByteToHex((value & 0xf000) >> 12);
		text[6] = halfByteToHex((value & 0x0f00) >>  8);
		text[7] = halfByteToHex((value & 0x00f0) >>  4);
		text[8] = halfByteToHex( value & 0x000f       );
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
