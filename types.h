
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

typedef double decimal;

// decimal sigmoid (decimal x) { return 1 / (1 + exp(-x)); }
// decimal dsigmoid(decimal x) { return x * (1 - x);       }

#define RAD(deg) ((deg) * 0.01745329)
#define DEG(rad) ((rad) * 57.29578  )

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
	unsigned int softleft;
	unsigned int softright;
};

struct Physics {
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
	decimal weight;
};

struct Node {
	unsigned int vallen;
	decimal val;
	decimal bias;
	unsigned int destlen;
	struct Nodeconnection *dest;
};

struct Car {
	unsigned int id;
	unsigned int sprite;
	unsigned int alive;
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
	unsigned int maxroadval;
	decimal fitness;
	unsigned int nodelen;
	struct Node *node;
};

struct Thread {
	pthread_t thread;
	unsigned int id;
	struct Car *start;
	unsigned int size;
};

struct Map {
	unsigned int x;
	unsigned int y;
	unsigned char* data;
};
