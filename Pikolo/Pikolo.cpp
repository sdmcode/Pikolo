// Pikolo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <iostream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

#include "Dungeon.h"
#include "ShaderProgram.h"

// MUST only be done ONCE 
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define INITIALISE_SUCCESS 1
#define INITIALISE_FAIL -1

// Keep track of frame rate
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Our window object pointer
GLFWwindow* window;

Dungeon* dungeon;
ShaderProgram* shader;

const float vertices[] = {
	// positions									 // texture coords
	(float)(TILE_SIZE / 2),  (float)(TILE_SIZE / 2), 0.0f,   1.0f, 1.0f, // top right
	(float)(TILE_SIZE / 2), (float)(-TILE_SIZE / 2), 0.0f,   1.0f, 0.0f, // bottom right
	(float)(-TILE_SIZE / 2), (float)(-TILE_SIZE / 2), 0.0f,   0.0f, 0.0f, // bottom left
	(float)(-TILE_SIZE / 2),  (float)(TILE_SIZE / 2), 0.0f,   0.0f, 1.0f  // top left 
};

const unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};

struct Camera {
	float posx;
	float posy;
};

typedef glm::vec2 Location;

Location location;

Camera camera;
glm::ivec2 cameraMovement;

glm::ivec2 movementVector;

float movementSpeed = 12.0f;
bool moving = false;

unsigned int VAO;
unsigned int VBO;
unsigned int EBO;

std::vector<unsigned int> textures;

unsigned int mapTexture;
int tileCountX, tileCountY;
double tileScaleX, tileScaleY;
int totalFrames;

std::vector<glm::mat4> tileFrameTransforms;

bool updatedCameraMovement = true;

static bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

glm::mat4 calcTileFrameTransform(int id)
{
	glm::mat4 mapScale = glm::scale(glm::mat4(1), glm::vec3(tileScaleX, tileScaleY, 0.0f));
	mapScale = glm::translate(mapScale, glm::vec3((double)(id % tileCountX), (double)(id / tileCountX), 0.0));
	
	return mapScale;
}

unsigned int loadTexture(std::string path)
{
	unsigned int tex;

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	
	if (data)
	{
		glGenTextures(1, &tex);

		if (path.find("map_x") != std::string::npos)
		{
			tileCountX = width / MAP_TILE_DIM;
			tileCountY = height / MAP_TILE_DIM;

#ifdef DEBUG_ON
			printf("Width=%d, Height=%d : nX=%d, nY=%d\n", width, height, tileCountX, tileCountY);
#endif

			tileScaleX = 1.0 / (double)(tileCountX);
			tileScaleY = 1.0 / (double)(tileCountY);

			totalFrames = tileCountX * tileCountY;

#ifdef DEBUG_ON
			printf("Calculating transforms for %d tiles.... Scale: %f, %f\n", totalFrames, tileScaleX, tileScaleY);
#endif

			for (int i = 0; i < totalFrames; i++)
			{
				tileFrameTransforms.push_back(calcTileFrameTransform(i));
			}
			mapTexture = tex;

#ifdef DEBUG_ON
			printf("Map texture bound to %d, %d frames....\n", mapTexture, tileFrameTransforms.size());
#endif
		}

		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		totalTexturesLoaded++;
	}
	else
	{
#ifdef DEBUG_ON
		printf("FAILED TO LOAD TEXTURE\n");
#endif
		return 0;
	}

	stbi_image_free(data);

	return tex;
}

void initTextures(std::string path)
{
#ifdef DEBUG_ON
	printf("Loading textures...\n");
#endif

	int count = 0;

	for (auto & dir : fs::recursive_directory_iterator(path))
	{

		if (!fs::is_directory(dir))
		{
			std::string str(dir.path().string());

			if (!endsWith(str, ".png"))
				continue;
			
#ifdef DEBUG_ON
			printf("Loading texture: ");
			printf(str.c_str());
			printf("\n");
#endif
			textures.push_back(loadTexture(dir.path().string()));
		}
	}
#ifdef DEBUG_ON
	printf("Loaded %d textures...\n", textures.size());
#endif
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{ 
	glViewport(0, 0, width, height);
}

void moveCamera(int x, int y)
{
	float newX = camera.posx + x;
	float newY = camera.posy + y;

	if (newX >= 0 && newY >= 0 && newX <= dungeon->getMaxDimension() && newY <= dungeon->getMaxDimension())
	{
		camera.posx = newX;
		camera.posy = newY;

		updatedCameraMovement = true;
	}
}

void moveCamera(glm::ivec2 moveDir)
{
	moveCamera(moveDir.x, moveDir.y);
}

void processInput(GLFWwindow *window)
{
	// ESCAPE!!
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		movementVector.x = -1;
		movementVector.y = 0;
		moving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		movementVector.x = 1;
		movementVector.y = 0;
		moving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		movementVector.x = 0;
		movementVector.y = -1;
		moving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		movementVector.x = 0;
		movementVector.y = 1;
		moving = true;
	}
}

void generateDungeon()
{
	dungeon = new Dungeon(182);

	dungeon->generate();
}

int initialise()
{
#ifdef DEBUG_ON
	printf("Initialising GL....\n");
#endif
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GL_MULTISAMPLE, 25);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Pikolo", NULL, NULL);

	if (window == NULL)
	{
		printf("Failed to initialise OpenGL window - Arborting\n");
		return INITIALISE_FAIL;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD - Aborting\n");
		return INITIALISE_FAIL;
	}

	glViewport(0, 0, WIDTH, HEIGHT);

	// This shouldn't do anything, since GLFW_RESIZABLE = FALSE
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// nice dull blue colour
	glClearColor(0.4f, 0.0f, 1.0f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
		
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	
	return INITIALISE_SUCCESS;
}

int initShaders()
{

#ifdef DEBUG_ON
	printf("Generating shader....\n");
#endif
	shader = new ShaderProgram();

	shader->initFromFiles("./res/shaders/vertex.vs", "./res/shaders/fragment.fs");	
	
	//shader->addUniform("colour");	

	shader->addUniform("transform");
	shader->addUniform("frame");

	return 1;
}

glm::mat4 calcTransform(Tile t)
{	
	glm::mat4 mat = glm::scale(glm::mat4(1), scale); 
	mat = glm::translate(mat, glm::vec3((t.posX * TILE_SIZE) - camera.posx, (t.posY * TILE_SIZE) - camera.posy, 0.0f));	 
	return mat;
}

std::vector<Tile> visibleTiles;
void render(float delta)
{
	glClear(GL_COLOR_BUFFER_BIT);

	shader->use();

	glBindVertexArray(VAO);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, mapTexture);

	for (Tile t : visibleTiles)
	{
		shader->setUniform("transform", calcTransform(t));

		shader->setUniform("frame", tileFrameTransforms[t.id % totalFrames]);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);

	shader->disable();
}

void resetMovement()
{
	moving = false;

	movementVector.x = 0;
	movementVector.y = 0;
}

void updateCamera(float delta)
{
	float deltaX = location.x - camera.posx;
	float deltaY = location.y - camera.posy;

	moveCamera(deltaX, deltaY);	
}


void update(float delta)
{
	float newX = location.x + movementSpeed * delta * movementVector.x;
	float newY = location.y + movementSpeed * delta * movementVector.y;

	if (newX >= 0 && newY >= 0 && newX <= dungeon->getMaxDimension() && newY <= dungeon->getMaxDimension()) {
		location.x = newX;
		location.y = newY;
	}
}

void gameLoop()
{
#ifdef DEBUG_ON	
	printf("Entering game loop...\n");
#endif

	float currentFrame;
	
#ifdef DEBUG_ON	
	double time = 0.0;
	int frames = 0;
#endif
	
	while (!glfwWindowShouldClose(window))
	{
		currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

#ifdef DEBUG_ON		
		time += deltaTime;
		frames++;

		if (time >= 1.0)
		{
			glfwSetWindowTitle(window, std::to_string(frames).c_str());

			frames = 0;
			time = 0;
		}
#endif

		processInput(window);		
		
		update(deltaTime / TARGET_UPDATE_TIME);
				
		updateCamera(deltaTime / TARGET_UPDATE_TIME);

		if (updatedCameraMovement) {
			visibleTiles = dungeon->getVisibleTiles(camera.posx, camera.posy);

			updatedCameraMovement = false;
		}

		render(deltaTime / TARGET_UPDATE_TIME);

		resetMovement();

		glfwSwapBuffers(window);

		glfwPollEvents();

	}
}


int main()
{
	if (initialise() != INITIALISE_SUCCESS)
	{
		return -1;
	}

	initShaders();

	initTextures("./res/textures/");

	generateDungeon();
	
	location = glm::vec2((float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
	camera = { (float)WIDTH / 2.0f, (float)HEIGHT / 2.0f };

	gameLoop();

	delete dungeon;
	delete shader;

	glfwTerminate();

    return 0;
}

