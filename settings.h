
#include "types.h"

// Terminal Settings
	#define SHOWBRAIN

// Map settins
	#define WINDOWX               680
	#define WINDOWY               480

	#define STARTX                90
	#define STARTY                450
	#define STARTDIR              270

	#define BARRIERCOLOR          0xFF111111
	#define ROADCOLOR             0xFF555555
	#define GRASSCOLOR            0xFF00DD00

	#define EYECOLOR              0xFFFF0000

	#define MAPPATH               "map.map"
	#define CARPATH               "./assets/cars.bmp"
	//#define FONTPATH              "./assets/font.bmp"
//

// Generation Settings
	#define GENERATIONSIZE        2000
	#define GENERATIONTIME        1000
	#define DEFAULTTICKSPERFRAME  1

	#define PROCESSES             6
//

// Physics Settings
	#define EYERESOLUTION         5 // The resolution of the eye raytracer
	#define MAXEYEVAL             40 // maxeyeval = MAXEYEVAL * EYERESOLUTION
	#define ROADFRICTION          0.7
	#define ACCEL                 3
	#define FORWARDACCEL          0.01 // (internal value used for calculation)
	#define WHEELDIRFRICTION      0.5
	#define WHEELDIRACCEL         20
	#define WHEELMAXDIR           30
	#define WHEELTURNBACK         3
//

// Neurel Network Settings
	/* 
		eye forward
		eye left
		eye right
		eye softleft
		eye softright
	*/ // Probably shoudn't change this
	#define INPUTNODES 5
	/*
		controller left
		controller right
		controller softleft
		controller softright
		controller forward
	*/ // Probably shoudn't change this
	#define OUTPUTNODES 3

	#define LAYERS 1
	#define NODESPERLAYER 4

	#define MUTATIONCHANCE 1
//
