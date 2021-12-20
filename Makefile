loinux:
	gcc main.c controller.c -s -g0 -o racer -lSDL2 -lm

winblows:
	gcc main.c controller.c -s -g3 -o racer.exe -l -ISDL2\include -LSDL2\lib -lmingw32 -lSDL2main -lSDL2

run:
	./racer
