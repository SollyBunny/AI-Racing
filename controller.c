
#include "types.h"
#include "controller.h"

/*
	The function carcontroller is called every physics cycle
	Here the AI should controll the car and do any other logic for the Ai
	It should be seperate and unable to cause change to the actuall simulation apart from controlls

	The output (controller) are
		car->controller.forward
		car->controller.left
		car->controller.right
	which should be set to either 1 or 0 depending if they should be changed

	The input (eyes)
		car->eyes.forward
		car->eyes.left
		car->eyes.right
	The position of car
		car->pos.x
		car->pos.y
	The speed of car
		car->vel.x
		car->vel.y
		car->forwardaccel
		car->wheeldir
	For any other information from the car you can find in types.h

	The function carinit is where any sort of AI stuff can be inited
	The neurel network can be stored in 
		car->node;
	where the length is
		car->nodelen

*/

// Init any sort of AI stuff here
void carinit(struct Car *car) {

	#define RANDOMBIAS() (decimal)(rand() % 2000 - 1000) / 1000

	car->nodelen = INPUTNODES + OUTPUTNODES + (LAYERS * NODESPERLAYER);
	car->node    = malloc(car->nodelen * sizeof(struct Node));

	for (unsigned int i = 0; i < car->nodelen; ++i) {
		car->node[i].destlen = 0;
	}
	
	for (unsigned int i = 0; i < INPUTNODES; ++i) {
		car->node[i].destlen = NODESPERLAYER;
		car->node[i].dest    = malloc(NODESPERLAYER * sizeof(struct Nodeconnection));
		for (unsigned int m = 0; m < NODESPERLAYER; ++m) {
			car->node[i].dest[m].i    = INPUTNODES + m;
			car->node[i].dest[m].bias = RANDOMBIAS();
		}
	}

	

	for (unsigned int i = 0; i < LAYERS - 1; ++i) {
		for (unsigned int m = 0; m < NODESPERLAYER; ++m) {
			car->node[INPUTNODES + i * NODESPERLAYER + m].destlen = NODESPERLAYER;
			car->node[INPUTNODES + i * NODESPERLAYER + m].dest    = malloc(NODESPERLAYER * sizeof(struct Nodeconnection));
			for (unsigned int j = 0; j < NODESPERLAYER; ++j) {
				car->node[INPUTNODES + i * NODESPERLAYER + m].dest[j].i    = i * NODESPERLAYER + NODESPERLAYER + INPUTNODES + j;
				car->node[INPUTNODES + i * NODESPERLAYER + m].dest[j].bias = RANDOMBIAS();
			}
		}
	}

	for (unsigned int i = INPUTNODES + ((LAYERS - 1) * NODESPERLAYER); i < INPUTNODES + OUTPUTNODES + ((LAYERS - 1) * NODESPERLAYER - 2); ++i) {
		for (unsigned int m = 0; m < NODESPERLAYER; ++m) {
			car->node[i + m].destlen = OUTPUTNODES;
			car->node[i + m].dest    = malloc(OUTPUTNODES * sizeof(struct Nodeconnection));
			for (unsigned int j = 0; j < OUTPUTNODES; ++j) {
				car->node[i + m].dest[j].i    = i + j + NODESPERLAYER;
				car->node[i + m].dest[j].bias = RANDOMBIAS();
			}
		}
	}
	
	// exit(0);
	
}

/*void carcontroller(struct Car *car) {

	car->controller.forward = 1;
	car->controller.left    = 0;
	car->controller.right   = 0;
	
}*/

/*void carcontroller(struct Car *car) {

	// A very bad ai i made just to test it works

	if (car->forwardvel < 1) {
		car->controller.forward = 1;
	} else {
		car->controller.forward = 0;
	}

	if (car->eyes.left == car->eyes.right) {
		car->controller.left    = 0;
		car->controller.right   = 0;
		// printf("NONE\n");
	} else if (car->eyes.left > car->eyes.right) {
		if (car->eyes.left < 10) {
			car->controller.left    = 1;
			car->controller.right   = 0;
			// printf("LEFT\n");
		}
	} else {
		if (car->eyes.right < 10) {
			car->controller.left    = 0;
			car->controller.right   = 1;
			// printf("RIGHT\n");
		}
	}
		
}*/

void carcontroller(struct Car *car) {

	#define EYENORMALIZE(a) (decimal)a

	car->node[0].val = EYENORMALIZE(car->eyes.left);
	car->node[1].val = EYENORMALIZE(car->eyes.right);
	car->node[2].val = EYENORMALIZE(car->eyes.forward);
	car->node[3].val = car->forwardvel;
	car->node[4].val = car->dir;

	for (unsigned int i = 5; i < car->nodelen; ++i) { 
		car->node[i].val = 0;
	}

	for (unsigned int i = 0; i < car->nodelen; ++i) {
		for (unsigned int m = 0; m < car->node[i].destlen; ++m) {
			car->node[car->node[i].dest[m].i].val += car->node[i].val * car->node[i].dest[m].bias;
			// if (car->node[i].val != 0) {
				// printf("%f\n", car->node[i].val * car->node[i].dest[m].bias);
			// }
		}
	}

	/*if (car->node[car->nodelen - 3].val > car->node[car->nodelen - 2].val) {
		car->controller.left  = 1;
		car->controller.right = 0;
	} else {
		car->controller.left  = 0;
		car->controller.right = 1;
	}*/ // lmao baby mode car control
	car->controller.left    = (car->node[car->nodelen - 3].val > 0) ? 1 : 0;
	car->controller.right   = (car->node[car->nodelen - 2].val > 0) ? 1 : 0;
	car->controller.forward = (car->node[car->nodelen - 1].val > 0) ? 1 : 0;
	// printf("%f\n", car->node[car->nodelen - 1].val);

}
