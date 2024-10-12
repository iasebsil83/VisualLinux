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
extern unsigned int   S2DE_width;
extern unsigned int   S2DE_height;
extern unsigned short S2DE_key;

//VisualLinux constants
#define WINDOW_DEFAULT_WIDTH  720
#define WINDOW_DEFAULT_HEIGHT 1280






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
	switch(event){
		case S2DE_DISPLAY:

			//memory
			displayCPUMems(DT__CPUMEMS_X, DT__CPUMEMS_Y, computer);
			displayRAM(DT__RAM_X, DT__RAM_Y, computer);

			//virtual screen
			displayScreen(DT__SCREEN_X, DT__SCREEN_Y, computer);
		break;

		case S2DE_KEYBOARD:
			switch(S2DE_key){

				//move forward just one step
				case S2DE_KEY_LEFT:
					operateCPU(computer);
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

	//create computer instance AND set also its static pointer to allow global access
	cpt* computer = computerAccess( newComputer(argv[1]) );

	//load kernel program
	loadKernel(computer);

	//launch window
	S2DE_init(argc,argv, "Visual Linux", WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
	S2DE_fullScreen();
	S2DE_start();

	//nothing went wrong
	return EXIT_SUCCESS;
}
