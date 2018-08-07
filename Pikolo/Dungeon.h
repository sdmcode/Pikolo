#pragma once

#include "stdafx.h"

struct Tile {

	unsigned int id;

	int posX;
	int posY;

	Colour colour = { 1, 1, 1 };

	Tile(int id, int x, int y)
	{
		this->id = id;

		posX = x;
		posY = y;
	}
};

class Dungeon
{
private:

	Dungeon() {}
	
public:
	Dungeon(int size);

	std::vector<Tile> getTiles();
	std::vector<Tile> getVisibleTiles(float camx, float camy);

	Dungeon generate();
	Dungeon generate(int seed);

	float getMaxDimension();
	
};