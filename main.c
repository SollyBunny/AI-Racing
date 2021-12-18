
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define RAD(deg) (deg * 0.01745329)
#define DEG(rad) (rad * 57.29578)

#define WINDOWX 680
#define WINDOWY 480

typedef double decimal;

void error(char str[]) {
	fputs("\x1b[1;31mError:\x1b[0m", stdout);
	fputs(str, stdout);
	putchar('\n');
	exit(1);
}

int powten(int a) {
	if (a == 0) return 1;
	int result = 1;
	for (; a > 1; --a) {
		result *= 10;
	}
	return result;
}

int intsize(int a) {
	if (a < 10) return 1;
	if (a < 100) return 2;
	if (a < 1000) return 3;
	if (a < 10000) return 4;
	else return 5;
}

struct Pos {
	decimal x;
	decimal y;
};

struct Controller {
	unsigned int forward;
	unsigned int left;
	unsigned int right;
};

struct Physics {
	decimal friction;
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
	struct Physics physics;
	struct Pos pos;
	struct Pos vel;
	decimal forwardvel;
	decimal dir;
	decimal wheeldir;
	decimal wheeldirvel;
};

void carclean(struct Car *car) {
	car->controller.forward = 0;
	car->controller.left = 0;
	car->controller.right = 0;
	car->physics.friction = 0.8;
	car->physics.accel = 5;
	car->physics.forwardaccel = 0.001;
	car->physics.wheeldirfriction = 0.5;
	car->physics.wheeldiraccel = 20;
	car->physics.wheelmaxdir = 30;
	car->physics.wheelturnback = 3;
	car->pos.x = 0;
	car->pos.y = 0;
	car->vel.x = 0;
	car->vel.y = 0;
	car->forwardvel = 0;
	car->wheeldir = 0;
	car->wheeldirvel = 0;
	car->dir = 0;
}

void carframe(struct Car *car) {
	// calculating x and y vel instead of dir & forward vel more accurate and allows sliding physics
	if (car->controller.forward) {
		car->vel.x += car->physics.accel * cos(RAD(car->dir));
		car->vel.y += car->physics.accel * sin(RAD(car->dir));
		car->forwardvel += car->physics.accel;
	}

	// not elseif, so if both are held they cancel out (prevent ai exploit :P)
	if (car->controller.left) {
		car->wheeldirvel -= car->physics.wheeldiraccel;
	}
	if (car->controller.right) {
		car->wheeldirvel += car->physics.wheeldiraccel;
	}


	// simulate speed of wheel turn
		car->wheeldir += car->wheeldirvel;
		car->wheeldirvel *= car->physics.wheeldirfriction;
	
	// stop wheels from turning 360
		if (car->wheeldir > car->physics.wheelmaxdir) car->wheeldir = car->physics.wheelmaxdir;
		if (car->wheeldir < -car->physics.wheelmaxdir) car->wheeldir = -car->physics.wheelmaxdir;

	// make wheel turn back to 0 degrees gradually
		car->wheeldir /= car->physics.wheelturnback;

	// simulate dir change from moving forward when wheels are turned
		car->dir += car->wheeldir * car->forwardvel / 10;
		if      (car->dir < 0  ) car->dir += 360;
		else if (car->dir > 360) car->dir -= 360;

	// move forward car cuz of its momentum
		car->pos.x += car->vel.x;
		car->pos.y += car->vel.y;

	// debug bounds check
		if (car->pos.x < 0) {
			car->vel.x *= -1;
			car->pos.x = 0;
		} else if (car->pos.x > WINDOWX - 65) {
			car->vel.x *= -1;
			car->pos.x = WINDOWX - 65;
		}

		if (car->pos.y < 0) {
			car->vel.y *= -1;
			car->pos.y = 0;
		} else if (car->pos.y > WINDOWY - 50) {
			car->vel.y *= -1;
			car->pos.y = WINDOWY - 50;
		}

	// dampen velocity cuz eee
		car->vel.x *= car->physics.friction;
		car->vel.y *= car->physics.friction;

	// dampen velocity of calculation speed
		car->forwardvel *= car->physics.friction;
	
}

int main(void) {
	// Init vars

		SDL_Window   *m_window;
		SDL_Renderer *m_window_renderer;

		SDL_Surface *_tempbmp;
		SDL_Texture *car_asset;
		SDL_Texture *font_asset;

		decimal fps;
		long unsigned int fps_pretime = 0;
		long unsigned int fps_curtime = 0;

	// Init everything
		if(SDL_Init(SDL_INIT_VIDEO) < 0) error("Failed to initialize the SDL2 library");

		m_window = SDL_CreateWindow(
			"SDL2 Window",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOWX, WINDOWY,
			0
		);
		if(!m_window) error("Failed to create window\n");

		m_window_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED && SDL_HINT_RENDER_VSYNC);
		SDL_SetRenderDrawColor(m_window_renderer, 0, 255, 0, 255);
		if(!m_window_renderer) error("Failed to get the renderer from the window\n");

	// Init assets

		struct Car car;
		carclean(&car);
		car.sprite = 1;
	
		_tempbmp   = SDL_LoadBMP("./assets/cars.bmp");
		car_asset  = SDL_CreateTextureFromSurface(m_window_renderer, _tempbmp);
		SDL_FreeSurface(_tempbmp);

		SDL_Rect    *car_size = malloc(1 * sizeof(SDL_Rect));
		car_size->x = 0;
		car_size->y = 0;
		car_size->w = 65;
		car_size->h = 50;

		SDL_Rect    *car_pos = malloc(1 * sizeof(SDL_Rect));
		car_pos->x = 0;
		car_pos->y = 0;
		car_pos->w = 60;
		car_pos->h = 50;

		SDL_Point   *car_center = malloc(1 * sizeof(SDL_Point));
		car_center->x = car_pos->w / 2;
		car_center->y = car_pos->h / 2;

		_tempbmp   = SDL_LoadBMP("./assets/font.bmp");
		font_asset = SDL_CreateTextureFromSurface(m_window_renderer, _tempbmp);
		SDL_FreeSurface(_tempbmp);

		SDL_Rect    *font_size = malloc(1 * sizeof(SDL_Rect));
		font_size->x = 0;
		font_size->y = 0;
		font_size->w = 6;
		font_size->h = 6;

		SDL_Rect    *font_pos = malloc(1 * sizeof(SDL_Rect));
		font_pos->x = 0;
		font_pos->y = 0;
		font_pos->w = 12;
		font_pos->h = 12;

	
	// Main loop
		fps_pretime = SDL_GetPerformanceCounter();
		while (1) {

			// Events

				SDL_Event e;
				while (SDL_PollEvent(&e) > 0) {
					switch (e.type) {
						case SDL_QUIT:
							goto end;
						case SDL_KEYDOWN:
						case SDL_KEYUP:
							switch (e.key.keysym.sym) {
								case SDLK_a:
									car.controller.left = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								case SDLK_d:
									car.controller.right = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								case SDLK_w:
									car.controller.forward = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								default: break;
							}
							break;
					}
				}

				carframe(&car);

			// Render

				SDL_RenderClear(m_window_renderer);

				// render car
					car_size->x = car.sprite * 65;
					car_pos->x = (unsigned int)car.pos.x;
					car_pos->y = (unsigned int)car.pos.y;
		        	SDL_RenderCopyEx(m_window_renderer, car_asset, car_size, car_pos, car.dir, car_center, 0);
		        
				// Render fps
					for (int i = 1; i < intsize(fps) + 1; ++i) {
						font_size->x = (
							((int)fps / powten(i)) % 10
						) * 6;
						font_pos->x = (intsize(fps) - i) * 10;
			        	SDL_RenderCopy(m_window_renderer, font_asset, font_size, font_pos);
			        }
		        
		        SDL_RenderPresent(m_window_renderer);

			// Fps

				//printf("Fps: %d\n", fps);

				fps_curtime = SDL_GetPerformanceCounter();

				fps = 1 / ((decimal)(fps_curtime - fps_pretime) / SDL_GetPerformanceFrequency());
				printf("Fps: %f\n", car.dir);

				fps_pretime = SDL_GetPerformanceCounter();
				
				if (fps > 15) { // limiting to 20fps, then doing a physics frame every second, cuz with ai i cant have time based physics that would be jank
					SDL_Delay(1000 / 20 - 1 / fps);
				}	

			//
			
		}

	end:
		// Destroy assets
			SDL_DestroyTexture(car_asset);

		// Destroy Window
			SDL_DestroyRenderer(m_window_renderer);
		    SDL_DestroyWindow(m_window);
		    SDL_Quit();

		// Exit window
			printf("Window Closed\n");
			return 0;

}
