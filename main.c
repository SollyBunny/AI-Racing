
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "controller.h"

#define WINDOWX 680
#define WINDOWY 480

#define ROADCOLOR 0xFF555555
#define GRASSCOLOR 0xFF00DD00

void error(char str[]) {
	fputs("\x1b[1;31mError:\x1b[0m ", stdout);
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

void carinfo(struct Car *car) {
	printf("\x1b[2J\x1b[HPosition: (%f, %f)\nVelocity: (%f, %f)\nEyes: ←: %*d ↑: %*d → : %*d\nController: %s%s%s\n", 
		car->pos.x, car->pos.y,
		car->vel.x, car->vel.y,
		3, car->eyes.left,
		3, car->eyes.forward,
		3, car->eyes.right,
		car->controller.left    ? "← " : "  ",
		car->controller.forward ? " ↑ " : "   ",
		car->controller.right   ? "→ " : "  "
	);
	
}

void carclean(struct Car *car) {
	car->controller.forward = 0;
	car->controller.left = 0;
	car->controller.right = 0;
	car->eyes.resolution = 5;
	car->eyes.forward = 0;
	car->eyes.left = 0;
	car->eyes.right = 0;
	car->physics.grassfriction = 0.6;
	car->physics.roadfriction = 0.9;
	car->physics.accel = 1;
	car->physics.forwardaccel = 0.01;
	car->physics.wheeldirfriction = 0.5;
	car->physics.wheeldiraccel = 20;
	car->physics.wheelmaxdir = 30;
	car->physics.wheelturnback = 3;
	car->pos.x = WINDOWX / 2;
	car->pos.y = WINDOWY / 2;
	car->vel.x = 0;
	car->vel.y = 0;
	car->forwardvel = 0;
	car->wheeldir = 0;
	car->wheeldirvel = 0;
	car->dir = 0;
	car->skid = 0;
	car->skidlen = 0;
	car->aienabled = 1;
}

struct Map map;

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
		// if (car->pos.x < 0) {
			// car->vel.x *= -1;
			// car->pos.x = 0;
		// } else if (car->pos.x > WINDOWX - 65) {
			// car->vel.x *= -1;
			// car->pos.x = WINDOWX - 65;
		// }
// 
		// if (car->pos.y < 0) {
			// car->vel.y *= -1;
			// car->pos.y = 0;
		// } else if (car->pos.y > WINDOWY - 50) {
			// car->vel.y *= -1;
			// car->pos.y = WINDOWY - 50;
		// }

	// dampen velocity cuz eee
		// check if on road or grass
		if (MAPVALUE(car->pos.x, car->pos.y) == 0) {
			car->vel.x *= car->physics.grassfriction;
			car->vel.y *= car->physics.grassfriction;
			car->forwardvel *= car->physics.grassfriction;
		} else {
			car->vel.x *= car->physics.roadfriction;
			car->vel.y *= car->physics.roadfriction;
			car->forwardvel *= car->physics.roadfriction;
		}

	// Calculate eyes (how far from grass)

		struct Pos add;
		#define resetadd() add.x = 0;add.y = 0

		car->eyes.forward = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) != 0) {
			add.x += car->eyes.resolution * cos(RAD(car->dir));
			add.y += car->eyes.resolution * sin(RAD(car->dir));
			++car->eyes.forward;
		}

		car->eyes.right = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) != 0) {
			add.x -= car->eyes.resolution * sin(RAD(car->dir));
			add.y += car->eyes.resolution * cos(RAD(car->dir));
			++car->eyes.right;
		}

		car->eyes.left = 0;
		resetadd();
		while (MAPVALUE(
			car->pos.x + add.x, 
			car->pos.y + add.y
		) != 0) {
			add.x += car->eyes.resolution * sin(RAD(car->dir));
			add.y -= car->eyes.resolution * cos(RAD(car->dir));
			++car->eyes.left;
		}
		
		// printf("Forward: %d\nLeft: %d\nRight: %d\n",
			// car->eyes.forward,
			// car->eyes.left,
			// car->eyes.right
		// );

	// calculate skid marks
		if (car->skid > 0) {
			if (car->skidlen < 50 && car->skid > 30) {
				car->skidpos[car->skidlen].x = car->pos.x - (15 * cos(RAD(car->dir)));
				car->skidpos[car->skidlen].y = car->pos.y - (15 * sin(RAD(car->dir)));
				++car->skidlen;
			}
			--car->skid;
		} else if (car->forwardvel > 5) {
			decimal temp_dir = DEG(atan(car->vel.y / car->vel.x));
			if      (temp_dir < 0  ) temp_dir += 360;
			else if (temp_dir > 360) temp_dir -= 360;
			if (fabs(temp_dir - car->dir) > 180) {
			// if (1) {
				car->skid = 50;
				car->skidlen = 0;
			}
		}

	// Get next ticks controlls
		if (car->aienabled) carcontroller(car);
}

int main(void) {
	// Init vars

		// printf("%d\n", map_y);

		SDL_Window   *m_window;
		SDL_Renderer *m_window_renderer;

		SDL_Surface *_tempbmp;
		SDL_Texture *car_asset;
		SDL_Texture *font_asset;
		SDL_Texture *map_asset;

		decimal fps;
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

			FILE *fp = fopen("map.map", "rb");
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
				} else {
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

		struct Car car;
		carclean(&car);
		car.sprite = 2;
	
		_tempbmp   = SDL_LoadBMP("./assets/cars.bmp");
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
		unsigned int tick = 0;
		while (1) {

			// Events

				SDL_Event e;
				while (SDL_PollEvent(&e) > 0) {
					switch (e.type) {
						case SDL_QUIT:
							goto end;
						case SDL_KEYDOWN:
							if (e.key.keysym.sym == SDLK_p) {
								car.controller.forward = 0;
								car.controller.left = 0;
								car.controller.right = 0;
								car.aienabled = car.aienabled == 0 ? 1 : 0;
								break;
							}
						case SDL_KEYUP:
							if (car.aienabled) break;
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

				if (tick == 20) {
					carinfo(&car);
					tick = 0;
				} else ++tick;
				
			// Render

				// SDL_SetRenderDrawColor(m_window_renderer, 0, 255, 0, 255);
				// SDL_RenderClear(m_window_renderer);

				// render map
					SDL_RenderCopy(m_window_renderer, map_asset, map_size, map_pos);

				// render skid
					if (car.skid > 0) {
						for (unsigned int i = 0; i < car.skidlen; ++i) {
							SDL_SetRenderDrawColor(m_window_renderer, 0, (i * 5), 0, 255);
							car_skid_mark->x = car.skidpos[i].x - (car_skid_mark->w / 2);
							car_skid_mark->y = car.skidpos[i].y - (car_skid_mark->h / 2);
							SDL_RenderFillRect(m_window_renderer, car_skid_mark);
						}
					}

				// render car
					car_size->x = car.sprite * 65 - 5;
					car_pos->x = (unsigned int)car.pos.x - car_pos->w / 2;
					car_pos->y = (unsigned int)car.pos.y - car_pos->h / 2;
		        	SDL_RenderCopyEx(m_window_renderer, car_asset, car_size, car_pos, car.dir, 0, 0);

				// render eyes
					SDL_SetRenderDrawColor(m_window_renderer, 255, 0, 0, 255);
					if (car.eyes.forward > 0) {
						SDL_RenderDrawLine(m_window_renderer,
							car.pos.x, car.pos.y,
							car.pos.x + car.eyes.forward * car.eyes.resolution * cos(RAD(car.dir)),
							car.pos.y + car.eyes.forward * car.eyes.resolution * sin(RAD(car.dir))
						);
					}
					if (car.eyes.right > 0) {
						SDL_RenderDrawLine(m_window_renderer,
							car.pos.x, car.pos.y,
							car.pos.x + car.eyes.right * car.eyes.resolution * -sin(RAD(car.dir)),
							car.pos.y + car.eyes.right * car.eyes.resolution * cos(RAD(car.dir))
						);
					}
					if (car.eyes.left > 0) {
						SDL_RenderDrawLine(m_window_renderer,
							car.pos.x, car.pos.y,
							car.pos.x + car.eyes.left * car.eyes.resolution * sin(RAD(car.dir)),
							car.pos.y + car.eyes.left * car.eyes.resolution * -cos(RAD(car.dir))
						);
					}

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

				fps_curtime = SDL_GetPerformanceCounter();

				fps = 1 / ((decimal)(fps_curtime - fps_pretime) / SDL_GetPerformanceFrequency());

				fps_pretime = SDL_GetPerformanceCounter();
				
				if (fps > 15) { // limiting to 20fps, then doing a physics frame every second, cuz with ai i cant have time based physics that would be jank
					SDL_Delay(1000 / 20 - 1 / fps);
				}	

			//
			
		}

	end:
		// Destroy assets
			SDL_DestroyTexture(car_asset);
			SDL_DestroyTexture(font_asset);
			SDL_DestroyTexture(map_asset);

		// Destroy Window
			SDL_DestroyRenderer(m_window_renderer);
		    SDL_DestroyWindow(m_window);
		    SDL_Quit();

		// Exit window
			printf("Window Closed\n");
			return 0;

	maperror:
		error("\"map.map\" is corrupt");

}
