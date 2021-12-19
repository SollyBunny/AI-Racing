#define RAD(deg) (deg * 0.01745329)
#define DEG(rad) (rad * 57.29578)

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

struct Car {
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
	unsigned int skid;
	unsigned int skidlen;
	struct Pos skidpos[50];
	unsigned int aienabled;
};

struct Map {
	unsigned int x;
	unsigned int y;
	unsigned char* data;
};
