#include <SFML/Graphics.hpp>

class Map {
public:
	sf::Vector2<int> size;	
	int datasize;
	sf::Vector2f* data;
};

Map readmap(char *name);
