#include <SFML/Graphics.hpp>

class Map {
public:
	sf::Vector2<int> size;	
	int datasize;
	sf::Vector2<int>* data;
};

Map readmap(char *name);
