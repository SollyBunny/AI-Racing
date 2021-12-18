#include <SFML/Graphics.hpp>
#include <cmath>
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

	float grad1, grad2, ang;
	grad2 = abs(map.data[map.datasize - 2].y - map.data[map.datasize - 1].y) / abs(map.data[map.datasize - 2].x - map.data[map.datasize - 1].x);
	for (int i = 1; i < map.datasize; ++i) {

		grad1 = fabs(map.data[i].y - map.data[i - 1].y) / fabs(map.data[i].x - map.data[i - 1].x);
		ang = atan(fabs(grad1 - grad2) / fabs(1 + (grad1 * grad2)));

		roadlines[i * 4 + 0].position.x = map.data[i].x - (outline * cos(ang));
		roadlines[i * 4 + 0].position.y = map.data[i].y - (outline * sin(ang));
		
		roadlines[i * 4 + 1].position.x = map.data[i].x - (outline * cos(ang));
		roadlines[i * 4 + 1].position.y = map.data[i].y + (outline * sin(ang));
		
		roadlines[i * 4 + 2].position.x = map.data[i - 1].x + (outline * cos(ang));
		roadlines[i * 4 + 2].position.y = map.data[i - 1].y + (outline * sin(ang));
		
		roadlines[i * 4 + 3].position.x = map.data[i - 1].x + (outline * cos(ang));
		roadlines[i * 4 + 3].position.y = map.data[i - 1].y - (outline * sin(ang));

		grad2 = grad1;
		
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
