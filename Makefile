main.o:
	g++ main.cpp map.cpp -o build/racing -O3 -s -lsfml-graphics -lsfml-window -lsfml-system
	./build/racing build/map.map
