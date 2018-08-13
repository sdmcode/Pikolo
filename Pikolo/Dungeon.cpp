#include "stdafx.h"

#include "Dungeon.h"
#include "PerlinNoise.h"

#include <math.h>

int roomSize = DEFAULT_ROOM_SIZE;

PerlinNoise _noise;
std::vector<Tile> tiles;

std::vector<Tile> Dungeon::getTiles()
{
	return tiles;
}

Box2d getBox(float posX, float posY, int sizeX, int sizeY)
{
	return {
		posX - (sizeX * TILE_SIZE / 2),
		posX + (sizeX * TILE_SIZE / 2),
		posY - (sizeY * TILE_SIZE / 2),
		posY + (sizeY * TILE_SIZE / 2)
	};
}

Box2d getBox(Tile t)
{
	return {
		(float)t.posX - (TILE_SIZE / 2),
		(float)t.posX + (TILE_SIZE / 2),
		(float)t.posY - (TILE_SIZE / 2),
		(float)t.posY + (TILE_SIZE / 2)
	};
}

bool checkWithin(Box2d a, Box2d b)
{
	return (b.bottom >= a.bottom && b.top <= a.top && b.left >= a.left && b.right <= a.right);
}

bool checkCollision(Box2d a, Box2d b)
{
	if (checkWithin(a, b) || checkWithin(b, a))
		return true;

	if ((b.left >= a.right && a.right <= b.right) && (b.bottom >= a.bottom && a.top >= b.bottom))
		return true;

	if ((b.left >= a.left && a.right >= b.left) && (b.top >= a.bottom && a.bottom >= b.bottom))
		return true;

	if ((b.right >= a.left && b.right <= a.right) && (b.top >= a.bottom && b.top <= a.top))
		return true;

	if ((b.right >= a.left && b.right <= a.right) && (b.bottom >= a.bottom && b.bottom <= a.top))
		return true;

	return false;
}

bool checkCollision(float posX, float posY, int sx, int sy)
{
	Box2d a = getBox(posX, posY, sx, sy);

	for (Tile t: tiles)
	{
		if (checkCollision(a, getBox(t)))
			return true;
	}
	return false;
}

std::vector<Tile> Dungeon::getVisibleTiles(float camx, float camy)
{
	std::vector<Tile> t;

	int firstX = (int)std::floor((camx / 64.0)) - TILES_ON_SCREEN_X;
	int firstY = (int)std::floor((camy / 64.0)) - TILES_ON_SCREEN_Y;

	int x, y, z;

	for (int j = -1; j <= TILES_ON_SCREEN_Y * 2 + 2; j++)
	{
		for (int i = -1; i <= TILES_ON_SCREEN_X * 2 + 1; i++)
		{
			x = firstX + i;
			y = firstY + j;

			z = (y * roomSize) + x;

			if (z >= 0 && z <= tiles.size() - 1) {
				t.push_back(tiles[z]);
			}
		}
	}

	return t;
}

Dungeon::Dungeon(int size)
{
	roomSize = size;

	if (roomSize > MAX_ROOM_SIZE)
		roomSize = MAX_ROOM_SIZE;
}

Dungeon Dungeon::generate()
{
	return generate(13375);
}

float Dungeon::getMaxDimension()
{
	return (roomSize - 1) * 64.0f;
}

Dungeon Dungeon::generate(int seed)
{
#ifdef DEBUG_ON
	printf("Generating room....\n");
#endif

	_noise.reseed(seed);

	if (roomSize > MAX_ROOM_SIZE)
		roomSize = MAX_ROOM_SIZE;

	int id = 0;

	for (int j = 0; j < roomSize - 1; j++)
	{
		for (int i = 0; i < roomSize; i++)
		{
			Tile t = { id++, i, j };
			
			tiles.push_back(t);
		}
	}

#ifdef DEBUG_ON
	printf("Finished generating dungeon...<size=%d>\n", roomSize);
#endif

	return *this;
}
