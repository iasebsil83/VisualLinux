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
	S2DE_setColor(0,0,0);
	S2DE_rectangle(
		x - DT__DATA_BOX_HALFWIDTH                                 , y-DT__DATA_BOX_HALFHEIGHT,
		x + DT__DATA_BOX_HALFWIDTH + DT__DATA_BOX_ASSUMED_TEXTWIDTH, y+DT__DATA_BOX_HALFHEIGHT,
		false
	);
	S2DE_text(hexText, DT__NUMBER_SIZE, x,y);
}

//program lines
void displayInstructions(int x, int y, cpt* computer) {

	//draw title
	S2DE_setColor(0, 0, 255);
	S2DE_text("Instructions", DT__TEXT_SIZE, x, y + DT__INSTRUCTION_BOX_HALFHEIGHT + DT__TITLE_HEIGHT_OFFSET);

	//draw instructions
	int originalY = y;
	for(ulng i=0ULL; i < CPT__INSTRUCTIONS_LENGTH; i++) {
		instruction* current = computer->instructions[i];

		//shift on a new column if necessary
		if(i != 0 && i%DT__MAX_INSTRUCTIONS_PER_COLUMN == 0) {
			x += DT__DATA_BOX_WIDTH + DT__INSTRUCTION_BOX_WIDTH;
			y  = originalY;
		}

		//draw index box
		drawShort(x,y, i);

		//draw instruction box
		int shiftedX = x + DT__DATA_BOX_ASSUMED_TEXTWIDTH + DT__DATA_BOX_HALFWIDTH + DT__INSTRUCTION_BOX_HALFWIDTH;
		S2DE_setColor(255, 0, 255);
		S2DE_rectangle(
			shiftedX - DT__INSTRUCTION_BOX_HALFWIDTH                                        , y-DT__INSTRUCTION_BOX_HALFHEIGHT,
			shiftedX + DT__INSTRUCTION_BOX_HALFWIDTH + DT__INSTRUCTION_BOX_ASSUMED_TEXTWIDTH, y+DT__INSTRUCTION_BOX_HALFHEIGHT,
			false
		);
		S2DE_setColor(0, 0, 0);
		if(current == NULL) {
			S2DE_text("#######", DT__TEXT_SIZE, shiftedX,y);
		} else {
			S2DE_text(current->text, DT__TEXT_SIZE, shiftedX,y);
		}

		//shift down for the next one
		y -= DT__INSTRUCTION_BOX_HEIGHT;
	}
}

//CPU
void displayCPUMems(int x, int y, cpt* computer) {
	int originalY         = y;
	char registerTitle[3] = "RX";

	//draw title
	S2DE_setColor(0, 0, 255);
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
	S2DE_setColor(0, 0, 255);
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

		//shift down for the next one
		y -= DT__DATA_BOX_HEIGHT;
	}
}

//virtual screen
void displayScreen(int x, int y, char* text) {

	//draw black screen
	S2DE_setColor(0, 0, 0);
	S2DE_rectangle(
		x-DT__SCREEN_HALFWIDTH, y-DT__SCREEN_HALFHEIGHT,
		x+DT__SCREEN_HALFWIDTH, y+DT__SCREEN_HALFHEIGHT,
		true
	);

	//draw yellow stroke
	S2DE_setColor(255, 255, 0);
	S2DE_rectangle(
		x-DT__SCREEN_HALFWIDTH, y-DT__SCREEN_HALFHEIGHT,
		x+DT__SCREEN_HALFWIDTH, y+DT__SCREEN_HALFHEIGHT,
		false
	);
}
