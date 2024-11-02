// ---------------- IMPORTATIONS ----------------

//standard
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

//computer structures
#include "computer.h"

//graphics
#include "graphics/S2DE.h"
#include "graphics/displayTools.h" //additionnal tools






// ---------------- DECLARATIONS ----------------

//S2DE variables
extern unsigned int   S2DE_width;
extern unsigned int   S2DE_height;
extern          int   S2DE_keyState;
extern unsigned short S2DE_key;
extern int            S2DE_background_red;
extern int            S2DE_background_green;
extern int            S2DE_background_blue;

//program constants
#define WINDOW_TITLE_TEXT "Visual Linux"






// ---------------- KERNEL / USER ----------------

//computer instance (both setter & getter)
#define GET NULL
cpt* computerAccess(cpt* access) {
	static cpt* computer;
	if(access != GET) {
		computer = (cpt*)access; //either SET & GET
	}
	return computer; //or only GET
}






// ---------------- EVENTS ----------------

//events
void S2DE_event(int event){
	cpt* computer = computerAccess(GET);
	const ubyt NONE = '\x00';
	const ubyt LEFT = '\x01';
	const ubyt RIGHT = '\x02';
	static ubyt moving = NONE;
	switch(event){
		case S2DE_DISPLAY:
			S2DE_setThickness(DT__GLOBAL_THICKNESS);

			//memory
			displayCPUMems(DT__CPUMEMS_X, DT__CPUMEMS_Y, computer);
			displayRAM(DT__RAM_X, DT__RAM_Y, computer);

			//virtual screen
			displayScreen(DT__SCREEN_X, DT__SCREEN_Y, computer);

			//current instruction
			displaySupervisedInstruction(DT__CURRENT_INSTRUCTION_X, DT__CURRENT_INSTRUCTION_Y, computer);
		break;

		case S2DE_KEYBOARD:
			switch(S2DE_keyState) {

				//pressed
				case S2DE_KEY_PRESSED:
					switch(S2DE_key){

						//move backward one step
						case S2DE_KEY_LEFT: case S2DE_KEY_UP: moving = LEFT; break;

						//move forward one step
						case S2DE_KEY_RIGHT: case S2DE_KEY_DOWN: moving = RIGHT; break;

						//execute current instruction
						case S2DE_KEY_RETURN: cpt__operateCPU(computer); S2DE_refresh(); break;
					}
				break;

				//released
				case S2DE_KEY_RELEASED:
					switch(S2DE_key){
						case S2DE_KEY_LEFT:  case S2DE_KEY_UP:
						case S2DE_KEY_RIGHT: case S2DE_KEY_DOWN: moving = NONE; break;
					}
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
			switch(moving) {
				case LEFT:
					if(computer->supervisionIndex > 0) { computer->supervisionIndex--; S2DE_refresh(); }
					usleep(100000);
				break;
				case RIGHT:
					if(computer->supervisionIndex < CPT__RAM_LENGTH-1) { computer->supervisionIndex++; S2DE_refresh(); }
					usleep(100000);
				break;
			}
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

	//check endianness
	if(isBigEndian()) {
		fputs("This program does not work on big endian machines.\n", stderr);
		exit(EXIT_FAILURE);
	}

	//create computer instance AND set also its static pointer to allow global access
	cpt* computer = computerAccess( newComputer(argv[1]) );

	//load kernel program
	cpt__loadKernel(computer);

	//tmp
	cpt__writeOnScreen(computer, "Hello");
	cpt__writeOnScreen(computer, "My name is Seb");
	cpt__writeOnScreen(computer, "Nice to meeet you");
	cpt__writeOnScreen(computer, "I am a screen");
	cpt__writeOnScreen(computer, "You can use me as I use quite long words");
	cpt__writeOnScreen(computer, "One two, one two...");
	cpt__writeOnScreen(computer, "This is a test");
	cpt__writeOnScreen(computer, "What a wonderful night it is");
	cpt__writeOnScreen(computer, "visualizing Linux kernel exe");
	cpt__writeOnScreen(computer, "in a more more visual way !");
	cpt__writeOnScreen(computer, "Hello");
	cpt__writeOnScreen(computer, "My name is Seb");
	cpt__writeOnScreen(computer, "Nice to meeet you");
	cpt__writeOnScreen(computer, "I am a screen");
	cpt__writeOnScreen(computer, "You can use me as I use quite long words");
	cpt__writeOnScreen(computer, "One two, one two...");
	cpt__writeOnScreen(computer, "This is a test");
	cpt__writeOnScreen(computer, "What a wonderful night it is");
	cpt__writeOnScreen(computer, "visualizing Linux kernel exe");
	cpt__writeOnScreen(computer, "in a more more visual way !");

	//a bit of aesthetics
	S2DE_background_red   = DT__COLOR_BACKGROUND_R;
	S2DE_background_green = DT__COLOR_BACKGROUND_G;
	S2DE_background_blue  = DT__COLOR_BACKGROUND_B;

	//launch window
	S2DE_init(argc,argv, WINDOW_TITLE_TEXT, DT__WINDOW_WIDTH, DT__WINDOW_HEIGHT);
	S2DE_fullScreen();
	S2DE_setTimer(30);
	S2DE_start();

	//nothing went wrong
	return EXIT_SUCCESS;
}
