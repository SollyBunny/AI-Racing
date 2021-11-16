#include <SFML/Graphics.hpp>
#include <iostream>
#include "map.hpp"

int main(int argc, char *argv[]) {

	float outline = 10;

	if (argc == 1) {
		std::cout << "Missing map file arg\n";
		return 1;
	} else if (argc > 2) {
		std::cout << "Too many args!\n";
		return 1;
	}
	Map map;
	map = readmap(argv[1]);

	sf::VertexArray roadlines(sf::Quads, map.datasize * 4);
	sf::CircleShape roadcirc = sf::CircleShape(outline);

	for (int i = 1; i < map.datasize; ++i) {

		roadlines[i * 4 + 0].position.x = (float)map.data[i].x;
		roadlines[i * 4 + 0].position.y = (float)map.data[i].y - outline;
		roadlines[i * 4 + 2].position.x = (float)map.data[i].x ;
		roadlines[i * 4 + 2].position.y = (float)map.data[i].y + outline;
		roadlines[i * 4 + 1].position.x = (float)map.data[i - 1].x;
		roadlines[i * 4 + 1].position.y = (float)map.data[i - 1].y;
		roadlines[i * 4 + 3].position.x = (float)map.data[i - 1].x;
		roadlines[i * 4 + 3].position.y = (float)map.data[i - 1].y;
		
	}
	
	

    sf::RenderWindow window(sf::VideoMode(400, 400), "Racing");
	window.setFramerateLimit(60);
	
    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            	case sf::Event::Closed:
	                window.close();
	            	break;
	            default:
	            	break;
	        }
        }

        window.clear(sf::Color::Black);

        window.draw(roadlines);
        for (int i = 0; i < map.datasize; ++i) {
        	roadcirc.setPosition(
        		(float)map.data[i].x - outline,
        		(float)map.data[i].y - outline
        	);
        	// window.draw(roadcirc);
        }

        window.display();
    }

    return 0;
}
