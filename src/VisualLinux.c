// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//computer structures
#include "computer.h"

//graphics
#include "graphics/S2DE.h"
#include "graphics/displayTools.h" //additionnal tools






// ---------------- DECLARATIONS ----------------

//S2DE variables
extern unsigned int S2DE_width;
extern unsigned int S2DE_height;

//VisualLinux constants
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1080






// ---------------- KERNEL / USER ----------------

//computer instance
cpt* getComputer() {
	static cpt* computer;
	return computer;
}
void initComputer() {
	cpt* c = malloc(sizeof(cpt));
	c->programLines = malloc(sizeof(ulng));
	c->ram          = malloc(sizeof(ulng));
	return c;
}

//run next instruction only
void runNext() {
	cpt* computer = getComputer();

	//get to the next instruction
	operateCPU();
}






// ---------------- EVENTS ----------------

//events
void S2DE_event(int event){
	switch(event){
		case S2DE_DISPLAY:

			//white background
			S2DE_setColor(255,255,255);
			S2DE_rectangle(0,0, S2DE_width,S2DE_height, true);

			//current program lines
			displayProgram();

			//memory
			displayCPURegisters();
			displayRAM();

			//virtual screen
			displayScreen();
		break;

		case S2DE_KEYBOARD:
			switch(S2DE_key){

				//move forward just one step
				case S2DE_KEY_LEFT:
					runNext();
				break;
			}
		break;

		case S2DE_MOUSE_CLICK:
		break;
		case S2DE_MOUSE_MOVE:
		break;
		case S2DE_RESIZE:
		break;
		case S2DE_TIMER:
		break;
	}
}






// ---------------- EXECUTION ----------------

//main
int main(int argc, char **argv) {

	//args
	if(argc <= 1) {
		fputs("VisualLinux: Missing first argument <storage_dir>.\n", stderr);
		exit(EXIT_FAILURE);
	}

	//try loading kernel
	cpt* computer = newComputer();
	loadKernel(argv[1], computer->programLines);

	//launch window
	S2DE_init(argc,argv, "Visual Linux", WINDOW_WIDTH, WINDOW_HEIGHT);
	S2DE_start();

	//nothing went wrong
	return EXIT_SUCCESS;
}
