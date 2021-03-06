// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <stdio.h>
#include <tchar.h>

#include <vector>

#define DEBUG_ON

#define WIDTH 640
#define HEIGHT 480

#define TILE_SIZE 64

// width of tiles in .png file
#define MAP_TILE_DIM 32

// Minimum number of tiles per dimension in a room 
#define MIN_ROOM_SIZE 3
// Maximum number of tiles per dimension in a room 
#define MAX_ROOM_SIZE 128

#define DEFAULT_ROOM_SIZE 16

// Minimum number of portals a room can have 
#define MIN_CONNECTEDNESS 1
// Maximum number of portals a room can have
#define MAX_CONECTEDNESS 8

#define DEFAULT_CONNECTEDNESS 1

// Application will attempt to update logic 25 times per second
// Rendering will happen as often as possible
const float TARGET_UPDATES = 25.0;
const float TARGET_UPDATE_TIME = 1.0f / TARGET_UPDATES;

const float WIDTH1PX = 2.0f / (float)(WIDTH);
const float HEIGHT1PX = 2.0f / (float)(HEIGHT); 

const float SIZETILEX = (float)(TILE_SIZE) * WIDTH1PX;
const float SIZETILEY = (float)(TILE_SIZE) * HEIGHT1PX;

const glm::vec3 scale = glm::vec3(WIDTH1PX, HEIGHT1PX, 0.0f);

const int TILES_ON_SCREEN_X = (WIDTH / (TILE_SIZE * 2));
const int TILES_ON_SCREEN_Y = (HEIGHT / (TILE_SIZE * 2));

struct Box2d {
	float left;
	float right;
	float bottom;
	float top;
};

struct Colour {
	float r, g, b, w = 1.0f;
};

static int totalTexturesLoaded = 0;

// TODO: reference additional headers your program requires here
