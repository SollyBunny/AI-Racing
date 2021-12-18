#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>

#include "map.hpp"
// i dont remmebr how i do this in cpp, so uhh

Map readmap(char *name) {
	Map out;

	std::cout << name << "\n";
	FILE *file = fopen(name, "rb");

	out.size.x = getc(file) | getc(file) << 8;
	out.size.y = getc(file) | getc(file) << 8;
	out.datasize = getc(file) | getc(file) << 8;
	out.data = (sf::Vector2f*)malloc(out.datasize * sizeof(sf::Vector2f));// idk if this is 2x32 2x64 or 1x64
	std::cout << "Size: " << out.size.x << ", " << out.size.y << "\nData: (size of " << out.datasize << ")\n";
	for (int i = 0; i < out.datasize; ++i) {
		out.data[i].x = getc(file) | getc(file) << 8;
		out.data[i].y = getc(file) | getc(file) << 8;
		std::cout << out.data[i].x << ", " << out.data[i].y << "\n";
	}

	return out;
}
