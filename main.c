#ifdef _WIN32
	#include <windows.h>
	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
		#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
	#endif
	HANDLE windows_hConsole;
#endif

#include <SDL2/SDL.h>

#include "settings.h"
#include "controller.h"

unsigned int tick = 0;
unsigned int ticksperframe = DEFAULTTICKSPERFRAME;

struct Car * car;

struct Thread* threads;	
unsigned int   threadsdone = 0;

void error(char str[]) {
	#ifdef _WIN32
		fputs("Error: ", stdout);
	#else
		fputs("\x1b[1;31mError:\x1b[0m ", stdout);
	#endif
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

SDL_Window   *m_window;
SDL_Renderer *m_window_renderer;

struct Map map;

unsigned int maxfitnessid = 0;
decimal maxfitness = 0;
unsigned int generation = 0;
unsigned int carsalive = GENERATIONSIZE;

void carinfo(struct Car *car) {
	#if _WIN32
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD count;
		GetConsoleScreenBufferInfo(windows_hConsole, &csbi);
		DWORD cellCount;
		cellCount = csbi.dwSize.X * csbi.dwSize.Y;
		FillConsoleOutputCharacter(windows_hConsole, (TCHAR)' ', cellCount, (COORD){0, 0}, &count);
		FillConsoleOutputAttribute(windows_hConsole, csbi.wAttributes, cellCount, (COORD){0, 0}, &count);
		SetConsoleCursorPosition(windows_hConsole, (COORD){0, 0});
		printf("Tick: %d\nGeneration: %d\nCars Alive: %d\nId: %d\nFitness: %f Maxfitness: %f\nPosition: (%f, %f)\nVelocity: (%f, %f)\nEyes: <: %*d ^: %*d >: %*d\nController: %c%c%c\n", 
	#else
		printf("\x1b[2J\x1b[HTick: %d\nGeneration: %d\nCars Alive: %d\nId: %d\nFitness: %f Maxfitness: %f\nPosition: (%f, %f)\nVelocity: (%f, %f)\nEyes: ←: %*d ↑: %*d → : %*d\nController: %s%s%s\n", 
	#endif
		tick,
		generation,
		carsalive,
		car->id,
		car->fitness, maxfitness,
		car->pos.x, car->pos.y,
		car->vel.x, car->vel.y,
		3, car->eyes.left,
		3, car->eyes.forward,
		3, car->eyes.right,
	#if _WIN32
		car->controller.left    ? '<' : ' ',
		car->controller.forward ? '^' : ' ',
		car->controller.right   ? '>' : ' '
	#else
		car->controller.left    ? "← " : "  " ,
		car->controller.forward ? " ↑ ": "   ",
		car->controller.right   ? "→ " : "  "
	#endif
	);

	#ifdef SHOWBRAIN

		putchar('\n');
		for (unsigned int i = 0; i < car->nodelen; ++i) {
			printf("%d: %f dest:", i, car->node[i].val);
			for (unsigned int m = 0; m < car->node[i].destlen; ++m) {
				printf("%d:%f, ", car->node[i].dest[m].i, car->node[i].dest[m].weight);
			}
			putchar('\n');
			if (
				i == INPUTNODES - 1 ||
				i == NODESPERLAYER * LAYERS + INPUTNODES - 1 ||
				(i > INPUTNODES - 1 &&
					(i - INPUTNODES + 1) % NODESPERLAYER == 0
				)
			) {
				putchar('\n');
			}
		}

	#endif
	
}

void carclean(struct Car *car) {
	car->eyes.resolution          = EYERESOLUTION;
	car->physics.accel            = ACCEL;
	car->physics.forwardaccel     = FORWARDACCEL;
	car->physics.wheeldirfriction = WHEELDIRFRICTION;
	car->physics.wheeldiraccel    = WHEELDIRACCEL;
	car->physics.wheelmaxdir      = WHEELMAXDIR;
	car->physics.wheelturnback    = WHEELTURNBACK;
	car->physics.roadfriction     = ROADFRICTION;
	car->aienabled = 1;
}

void carresetpos(struct Car *car) {
	car->controller.forward = 0;
	car->controller.left    = 0;
	car->controller.right   = 0;
	// car->eyes.forward = 0;
	// car->eyes.left = 0;
	// car->eyes.right = 0;
	car->pos.x = STARTX;//WINDOWX / 2;
	car->pos.y = STARTY;//WINDOWY / 2;
	car->vel.x = 0;
	car->vel.y = 0;
	car->forwardvel = FORWARDACCEL;
	car->wheeldir = 0;
	car->wheeldirvel = 0;
	car->dir = STARTDIR;
	// car->skid = 0;
	// car->skidlen = 0;
	car->maxroadval = 0;
	car->fitness = 0;
	car->alive = 1;
}

void carframe(struct Car *car) {

	unsigned int _MAPVALUE_pos;
	#define MAPVALUE(_x, _y) ((_MAPVALUE_pos = (unsigned int)(_y) * map.x + (unsigned int)(_x)) > map.x * map.y ? 0 : map.data[_MAPVALUE_pos])

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

	// dampen velocity cuz eee
		// check if on road or grass
		_MAPVALUE_pos = MAPVALUE(car->pos.x, car->pos.y); // hehe janky
		if (_MAPVALUE_pos <= 1) {
			car->alive = 0;
			return;
		} else {
			car->vel.x      *= car->physics.roadfriction;
			car->vel.y      *= car->physics.roadfriction;
			car->forwardvel *= car->physics.roadfriction;
		}

		if (car->forwardvel < 0.00001) {
			car->alive = 0;
			return;
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

	// debug bounds check
		// if (car->pos.x < 0) {
		// 	car->vel.x *= -1;
		// 	car->pos.x = 0;
		// } else if (car->pos.x > WINDOWX - 30) {
		// 	car->vel.x *= -1;
		// 	car->pos.x = WINDOWX - 30;
		// }
		
		// if (car->pos.y < 0) {
		// 	car->vel.y *= -1;
		// 	car->pos.y = 0;
		// } else if (car->pos.y > WINDOWY - 30) {
		// 	car->vel.y *= -1;
		// 	car->pos.y = WINDOWY - 30;
		// }

	// move forward car cuz of its momentum
		car->pos.x += car->vel.x;
		car->pos.y += car->vel.y;

	// Calculate eyes (how far from grass)

		struct Pos add;
		#define resetadd() add.x = 0;add.y = 0

		car->eyes.forward = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) > 1) {
			add.x += car->eyes.resolution * cos(RAD(car->dir));
			add.y += car->eyes.resolution * sin(RAD(car->dir));
			if ((++car->eyes.forward) > MAXEYEVAL) break;
		}

		car->eyes.left = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) > 1) {
			add.x += car->eyes.resolution * sin(RAD(car->dir));
			add.y -= car->eyes.resolution * cos(RAD(car->dir));
			if ((++car->eyes.left) > MAXEYEVAL) break;
		}

		car->eyes.right = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) > 1) {
			add.x -= car->eyes.resolution * sin(RAD(car->dir));
			add.y += car->eyes.resolution * cos(RAD(car->dir));
			if ((++car->eyes.right) > MAXEYEVAL) break;
		}

		car->eyes.softleft = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) > 1) {
			add.x += car->eyes.resolution * sin(RAD(car->dir - 315));
			add.y -= car->eyes.resolution * cos(RAD(car->dir - 315));
			if ((++car->eyes.softleft) > MAXEYEVAL) break;
		}

		car->eyes.softright = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) > 1) {
			add.x -= car->eyes.resolution * sin(RAD(car->dir - 45));
			add.y += car->eyes.resolution * cos(RAD(car->dir - 45));
			if ((++car->eyes.softright) > MAXEYEVAL) break;
		}

	// calculate skid marks
		// if (car->skid > 0) {
			// if (car->skidlen < 50 && car->skid > 30) {
				// car->skidpos[car->skidlen].x = car->pos.x - (15 * cos(RAD(car->dir)));
				// car->skidpos[car->skidlen].y = car->pos.y - (15 * sin(RAD(car->dir)));
				// ++car->skidlen;
			// }
			// --car->skid;
		// } else if (car->forwardvel > 5) {
			// decimal temp_dir = DEG(atan(car->vel.y / car->vel.x));
			// if      (temp_dir < 0  ) temp_dir += 360;
			// else if (temp_dir > 360) temp_dir -= 360;
			// if (fabs(temp_dir - car->dir) > 180) {
			// // if (1) {
				// car->skid = 50;
				// car->skidlen = 0;
			// }
		// }

	// Get next ticks controlls
		// if (car->fitness > 0 && (car->forwardvel < 0.5)) {
			// car->fitness--;
		if (MAPVALUE(car->pos.x, car->pos.y) > car->maxroadval) {
			car->maxroadval = MAPVALUE(car->pos.x, car->pos.y);
			car->fitness += 5;
			car->fitness += car->forwardvel * 0.1;
		} else {
			car->fitness -= 0.1;
		}
		// car->fitness += car->forwardvel * 0.1;
		// if (MAPVALUE(car->pos.x, car->pos.y) == 0) {
			// car->fitness -= 0.1;
		// } else {
			// car->fitness = (
				// car->fitness + (
					// MAPVALUE(car->pos.x, car->pos.y)
				// ))
			// / 2;
		// }
		if (car->aienabled) carcontroller(car);
}

void carrender(struct Car *car, SDL_Texture *car_asset, SDL_Rect *car_size, SDL_Rect *car_pos) {

	// render car
	
		SDL_SetRenderDrawColor(m_window_renderer, 255, 255, 255, 255);
		car_pos->x = (unsigned int)car->pos.x - car_pos->w / 2;
		car_pos->y = (unsigned int)car->pos.y - car_pos->h / 2;
		if (car->id == maxfitnessid) {		
			car_size->x = 6 * 65 - 5;
		} else {
			car_size->x = car->sprite * 65 - 5;
		}
   		SDL_RenderCopyEx(m_window_renderer, car_asset, car_size, car_pos, car->dir, 0, 0);
		
	// render eyes

		SDL_SetRenderDrawColor(m_window_renderer, 
			(EYECOLOR >> 16) & 0xFF, // r
			(EYECOLOR >>  8) & 0xFF, // g
			(EYECOLOR >>  0) & 0xFF, // b
			(EYECOLOR >> 24) & 0xFF  // a
		); // -g3 lmao
		if (car->eyes.forward > 0) {
			SDL_RenderDrawLine(m_window_renderer,
				car->pos.x, car->pos.y,
				car->pos.x + car->eyes.forward * car->eyes.resolution * cos(RAD(car->dir)),
				car->pos.y + car->eyes.forward * car->eyes.resolution * sin(RAD(car->dir))
			);
		}
		if (car->eyes.left > 0) {
				SDL_RenderDrawLine(m_window_renderer,
					car->pos.x, car->pos.y,
					car->pos.x + car->eyes.left * car->eyes.resolution *  sin(RAD(car->dir)),
					car->pos.y + car->eyes.left * car->eyes.resolution * -cos(RAD(car->dir))
				);
			}
		if (car->eyes.right > 0) {
			SDL_RenderDrawLine(m_window_renderer,
				car->pos.x, car->pos.y,
				car->pos.x + car->eyes.right * car->eyes.resolution * -sin(RAD(car->dir)),
				car->pos.y + car->eyes.right * car->eyes.resolution *  cos(RAD(car->dir))
			);
		}
		if (car->eyes.softleft > 0) {
			SDL_RenderDrawLine(m_window_renderer,
				car->pos.x, car->pos.y,
				car->pos.x + car->eyes.softleft * car->eyes.resolution *  sin(RAD(car->dir - 315)),
				car->pos.y + car->eyes.softleft * car->eyes.resolution * -cos(RAD(car->dir - 315))
			);
		}
		if (car->eyes.softright > 0) {
			SDL_RenderDrawLine(m_window_renderer,
				car->pos.x, car->pos.y,
				car->pos.x + car->eyes.softright * car->eyes.resolution * -sin(RAD(car->dir - 45)),
				car->pos.y + car->eyes.softright * car->eyes.resolution *  cos(RAD(car->dir - 45))
			);
		}

}

void cargeneration(struct Car* car) {
	++generation;
	maxfitness = 0;
	for (unsigned int i = 0; i < GENERATIONSIZE; ++i) {
		if (car[i].fitness > maxfitness) {
			maxfitness = car[i].fitness;
			maxfitnessid = i;
		}
	}
	for (unsigned int i = 0; i < GENERATIONSIZE; ++i) { if (i != maxfitnessid) {

		car[i].fitness = 0;

		car[i].nodelen = car[maxfitnessid].nodelen;
		car[i].node = realloc(car[i].node, car[i].nodelen * sizeof(struct Node));
		for (unsigned int m = 0; m < car[i].nodelen; ++m) {
			car[i].node[m].destlen = car[maxfitnessid].node[m].destlen;

			if (car[i].node[m].destlen != 0) {
				car[i].node[m].bias    = car[maxfitnessid].node[m].bias;
				if (rand() % 100 <= MUTATIONCHANCE) {
					car[i].node[m].bias = SIGMOID(car[i].node[m].bias + RANDOMDECIMAL());
				}
				car[i].node[m].dest    = realloc(car[i].node[m].dest, car[i].node[m].destlen * sizeof(struct Nodeconnection));
				for (unsigned int j = 0; j < car[i].node[m].destlen; ++j) {
					car[i].node[m].dest[j].weight = car[maxfitnessid].node[m].dest[j].weight;
					if (rand() % 100 <= MUTATIONCHANCE) {
						car[i].node[m].dest[j].weight = SIGMOID(car[i].node[m].dest[j].weight + RANDOMDECIMAL());
					}
				}
			}
			
		}
		
	} }

}

void threadfunc(struct Thread *thread) {
	unsigned int tickstodo;
	if (ticksperframe >= GENERATIONTIME) {
		tickstodo = GENERATIONTIME;
	} else {
		tickstodo = ticksperframe;
	}
	unsigned int tempdeadcars = 0;
	unsigned int oldalivecars;
	for (unsigned int i = 0; i < tickstodo; ++i) {
		for (unsigned int m = 0; m < thread->size; ++m) {
			if ((thread->start + m)->alive) {
				carframe(thread->start + m);
				if ((thread->start + m)->alive == 0) { // car has died :P
					tempdeadcars++;
				}
			}
			// some janky pointer stuff going on here, lets hope it works
		}
		oldalivecars = carsalive - tempdeadcars;
		if (oldalivecars == 1) { // if only the best car is left, assume
			if (car[maxfitnessid].alive == 1) {
				carsalive = 0;
				return;
			}
		}
		do {
			carsalive -= tempdeadcars;
		} while (carsalive != oldalivecars);
		if (carsalive == 0) {
			return;
		}
	}
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	#ifdef _WIN32
		windows_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	    DWORD dwMode = 0;
	    if (!GetConsoleMode(windows_hConsole, &dwMode)){
	        fputs("Failed to get the console mode.\n", stderr);
	        exit(GetLastError());
	    }
	    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	    if (!SetConsoleMode(windows_hConsole, dwMode)) {
	        fputs("Failed to set the console mode.\n", stderr);
	        exit(GetLastError());
	    }
	#endif
	// Init vars

		#ifdef _WIN32
			srand(GetTickCount());
		#else
			struct timespec _t;
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &_t);
			srand(_t.tv_nsec);
		#endif
		
		SDL_Surface *_tempbmp;
		SDL_Texture *car_asset;
		// SDL_Texture *font_asset;
		SDL_Texture *map_asset;

		decimal fps;
		decimal maxfps = 20;
		long unsigned int fps_pretime = 0;
		long unsigned int fps_curtime = 0;

		decimal zoom = 1;

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
		if(!m_window_renderer) error("Failed to get the renderer from the window\n");

	// Init assets

		// Init Map

			FILE *fp = fopen(MAPPATH, "rb");
			unsigned char c;
			if (fp == NULL) error("Cannot load \"map.map\"");
			
			c = fgetc(fp); if (c == (unsigned char)EOF) goto maperror;
			map.x = c * 256;
			c = fgetc(fp); if (c == (unsigned char)EOF) goto maperror;
			map.x += c;
			c = fgetc(fp); if (c == (unsigned char)EOF) goto maperror;
			map.y = c * 256;
			c = fgetc(fp); if (c == (unsigned char)EOF) goto maperror;
			map.y += c;

			unsigned int i = 0;
			map.data = malloc(map.x * map.y * sizeof(unsigned char));
			while ( (c = fgetc(fp)) != (unsigned char)EOF ) {
				map.data[i] = c;
				++i;
			}
			if (i != map.x * map.y) {
				printf("I: %i\nC: %d", i, c);
				free(map.data);
				goto maperror;
			}

			map_asset = SDL_CreateTexture(
				m_window_renderer, 
				SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
				map.x, map.y
			);

			void *pixels;
			int pitch;
			Uint32 *dst;
			SDL_LockTexture(map_asset, NULL, &pixels, &pitch);
			dst = (Uint32*)(Uint8*)pixels;
			for (unsigned int i = 0; i < map.x * map.y; ++i) {
				if (map.data[i] == 0) {
					*dst++ = GRASSCOLOR;
				} else if (map.data[i] == 1) {
					*dst++ = BARRIERCOLOR;
				} else {
					// *dst++ = map.data[i] * 256;
					*dst++ = ROADCOLOR;
				}
			}
			SDL_UnlockTexture(map_asset);

		SDL_Rect    *map_size = malloc(1 * sizeof(SDL_Rect));
		map_size->x = 0;
		map_size->y = 0;
		map_size->w = WINDOWX;
		map_size->h = WINDOWY;

		SDL_Rect    *map_pos = malloc(1 * sizeof(SDL_Rect));
		map_pos->x = 0;
		map_pos->y = 0;
		map_pos->w = WINDOWX;
		map_pos->h = WINDOWY;

		// Init cars
			car = malloc(GENERATIONSIZE * sizeof(struct Car));
			for (unsigned int i = 0; i < GENERATIONSIZE; ++i) {
				carclean(&car[i]);
				carresetpos(&car[i]);
				carinit(&car[i]);
				car[i].sprite = rand() % 6;
				car[i].id = i;
			}
		
		_tempbmp   = SDL_LoadBMP(CARPATH);
		car_asset  = SDL_CreateTextureFromSurface(m_window_renderer, _tempbmp);
		SDL_FreeSurface(_tempbmp);

		SDL_Rect    *car_size = malloc(1 * sizeof(SDL_Rect));
		car_size->x = 0;
		car_size->y = 0;
		car_size->w = 64;
		car_size->h = 64;

		SDL_Rect    *car_pos = malloc(1 * sizeof(SDL_Rect));
		car_pos->x = 0;
		car_pos->y = 0;
		car_pos->w = car_size->w * zoom / 2;
		car_pos->h = car_size->h * zoom / 2;

		SDL_Rect    *car_skid_mark = malloc(1 * sizeof(SDL_Rect));
		car_skid_mark->x = 0;
		car_skid_mark->y = 0;
		car_skid_mark->w = 10;
		car_skid_mark->h = 10;

		// SDL_Point   *car_center = malloc(1 * sizeof(SDL_Point));
		// car_center.x = car_pos->w / 2;

		// _tempbmp   = SDL_LoadBMP();
		// font_asset = SDL_CreateTextureFromSurface(m_window_renderer, _tempbmp);
		// SDL_FreeSurface(_tempbmp);

		// SDL_Rect    *font_size = malloc(1 * sizeof(SDL_Rect));
		// font_size->x = 0;
		// font_size->y = 0;
		// font_size->w = 6;
		// font_size->h = 6;

		// SDL_Rect    *font_pos = malloc(1 * sizeof(SDL_Rect));
		// font_pos->x = 0;
		// font_pos->y = 0;
		// font_pos->w = 12;
		// font_pos->h = 12;

	// Init pthreads

		threads = malloc(PROCESSES * sizeof(struct Thread));
		for (unsigned int i = 0; i < PROCESSES; ++i) {
			threads[i].id    = i;
			threads[i].start = &car[i * (GENERATIONSIZE / PROCESSES)];
			threads[i].size  = GENERATIONSIZE / PROCESSES;
		} 
	
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
							switch (e.key.keysym.sym) {
								case SDLK_p:
									car[0].controller.forward = 0;
									car[0].controller.left = 0;
									car[0].controller.right = 0;
									car[0].aienabled = car[0].aienabled == 0 ? 1 : 0;
									break;
								case SDLK_1:
									ticksperframe = 1;
									break;
								case SDLK_2:
									ticksperframe = 5;
									break;
								case SDLK_3:
									ticksperframe = 10;
									break;
								case SDLK_4:
									ticksperframe = 50;
									break;
								case SDLK_5:
									tick = GENERATIONTIME;
									ticksperframe = GENERATIONTIME;
									break;
								case SDLK_6:
									tick = GENERATIONTIME;
									ticksperframe = GENERATIONTIME * 5;
									break;
								case SDLK_7:
									tick = GENERATIONTIME;
									ticksperframe = GENERATIONTIME * 10;
									break;
								case SDLK_0:
									tick = GENERATIONTIME;
									break;
								default: break;
							}
						case SDL_KEYUP:
							if (car[0].aienabled) break;
							switch (e.key.keysym.sym) {
								case SDLK_a:
									car[0].controller.left = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								case SDLK_d:
									car[0].controller.right = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								case SDLK_w:
									car[0].controller.forward = (e.type == SDL_KEYDOWN) ? 1 : 0;
									break;
								default: break;
							}
							break;
					}
				}

				for (unsigned int i = 0; i < 1 + (ticksperframe / GENERATIONTIME); ++i) {
					for (unsigned int m = 0; m < PROCESSES; ++m) {
						pthread_create(&threads[m].thread, NULL, (void *)threadfunc, &threads[m]);
					} 
					for (unsigned int m = 0; m < PROCESSES; ++m) {
						pthread_join(threads[m].thread, NULL);
					}
					if (carsalive == 0) {
						cargeneration(car);
					} else if (ticksperframe >= GENERATIONTIME) {
						cargeneration(car);
						if (i < (ticksperframe / GENERATIONTIME)) {
							for (unsigned int j = 0; j < GENERATIONSIZE; ++j) {
					        	carresetpos(&car[j]);
					        }
							carsalive = GENERATIONSIZE;
				        }
					} else {
						tick += ticksperframe;
						if (tick >= GENERATIONTIME) {
							cargeneration(car);
						}
					}
					//carinfo(&car[maxfitnessid]);

				}
				

				
			// Render

				// SDL_SetRenderDrawColor(m_window_renderer, 0, 255, 0, 255);
				// SDL_RenderClear(m_window_renderer);

				// render map
					SDL_RenderCopy(m_window_renderer, map_asset, map_size, map_pos);

				// render cars
					for (unsigned int i = 0; i < GENERATIONSIZE; ++i) { 
						if (i != maxfitnessid) {
							if (car[i].alive == 1) {
								carrender(&car[i], car_asset, car_size, car_pos);
							}
						}
					}
					carrender(&car[maxfitnessid], car_asset, car_size, car_pos); // Render best car regardless if its alive or not

				// Render fps
					// for (int i = 1; i < intsize(fps) + 1; ++i) {
						// font_size->x = (
							// ((int)fps / powten(i)) % 10
						// ) * 6;
						// font_pos->x = (intsize(fps) - i) * 10;
			        	// SDL_RenderCopy(m_window_renderer, font_asset, font_size, font_pos);
			        // }
		        
		        SDL_RenderPresent(m_window_renderer);

				#if _WIN32
					if (tick % 5 == 0) {
				#else
					if (tick % 20 == 0) { // the performance on cmd is garbaeg, so slow down the output
				#endif
					carinfo(&car[maxfitnessid]);
				}

				// reset pos after render
				if (carsalive == 0) goto resetpos;
				if (tick >= GENERATIONTIME) {
					tick = 0;
					resetpos:
						for (unsigned int i = 0; i < GENERATIONSIZE; ++i) {
							carresetpos(&car[i]);
						}
						carsalive = GENERATIONSIZE;
				}	

			// Fps

				fps_curtime = SDL_GetPerformanceCounter();

				fps = 1 / ((decimal)(fps_curtime - fps_pretime) / SDL_GetPerformanceFrequency());

				fps_pretime = SDL_GetPerformanceCounter();
				
				if (fps > maxfps - 5) { // limiting to 20fps, then doing a physics frame every second, cuz with ai i cant have time based physics that would be jank
					SDL_Delay(1000 / maxfps - 1 / fps);
				}	

			//
			
		}

	end:
		// Destroy assets
			SDL_DestroyTexture(car_asset);
			// SDL_DestroyTexture(font_asset);
			SDL_DestroyTexture(map_asset);

		// Destroy Window
			SDL_DestroyRenderer(m_window_renderer);
		    SDL_DestroyWindow(m_window);
		    SDL_Quit();

		// Free other stuff
			free(threads);
			// TODO free neurel network

		// Exit window
			printf("Window Closed\n");
			return 0;

	maperror:
		error("\"map.map\" is corrupt");

}
