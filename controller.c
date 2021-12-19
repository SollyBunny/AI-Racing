
#include <stdio.h>

#include "types.h"
#include "controller.h"

/*
	The function in this file is called every physics cycle
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

*/

/*void carcontroller(struct Car *car) {

	car->controller.forward = 1;
	car->controller.left    = 0;
	car->controller.right   = 0;
	
}*/

void carcontroller(struct Car *car) {

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
		
}
