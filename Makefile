loinux:
	gcc main.c -s -g3 -o racer -lSDL2 -lm

winblows:
	gcc main.c -s -g3 -o racer.exe -l -ISDL2\include -LSDL2\lib -lmingw32 -lSDL2main -lSDL2
