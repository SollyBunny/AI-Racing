
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define RAD(deg) (deg * 0.01745329)
#define DEG(rad) (rad * 57.29578)

# define GENERATIONSIZE 500
# define GENERATIONTIMEOUT 150

#define MAXEYEVAL 10

#define STARTX 80
#define STARTY 470
#define STARTDIR 270

#define WINDOWX 680
#define WINDOWY 480

#define BARRIERCOLOR  0xFF111111
#define ROADCOLOR     0xFF555555
#define GRASSCOLOR    0xFF00DD00

/* 
	eye left
	eye right
	eye forward
	dir
*/
#define INPUTNODES 3
/*
	controller left
	controller right
	controller forward
*/
#define OUTPUTNODES 3
#define LAYERS 2
#define NODESPERLAYER 5

#define MUTATIONCHANCE 50

typedef double decimal;

struct Pos {
	decimal x;
	decimal y;
};

struct Controller {
	unsigned int forward;
	unsigned int left;
	unsigned int right;
};

struct Eyes {
	decimal resolution;
	unsigned int forward;
	unsigned int left;
	unsigned int right;
};

struct Physics {
	decimal grassfriction;
	decimal roadfriction;
	decimal accel;
	decimal forwardaccel;
	decimal wheeldirfriction;
	decimal wheeldiraccel;
	decimal wheelmaxdir;
	decimal wheelturnback;
};

struct Nodeconnection {
	unsigned int i;
	decimal bias;
};

struct Node {
	unsigned int vallen;
	decimal val;
	unsigned int destlen;
	struct Nodeconnection *dest;
};

struct Car {
	unsigned int id;
	unsigned int sprite;
	struct Controller controller;
	struct Eyes eyes;
	struct Physics physics;
	struct Pos pos;
	struct Pos vel;
	decimal forwardvel;
	decimal dir;
	decimal wheeldir;
	decimal wheeldirvel;
	// unsigned int skid;
	// unsigned int skidlen;
	// struct Pos skidpos[50];
	unsigned int aienabled;
	unsigned int fitness;
	unsigned int nodelen;
	struct Node *node;
};

struct Map {
	unsigned int x;
	unsigned int y;
	unsigned char* data;
};
