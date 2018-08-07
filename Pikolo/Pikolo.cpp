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

//const glm::mat4 Projection = glm::ortho(0.0f, static_cast<float>(WIDTH), 
//									0.0f, static_cast<float>(HEIGHT));

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

Camera camera;

float cameraSpeed = 20.0f;

unsigned int VAO;
unsigned int VBO;
unsigned int EBO;

std::vector<unsigned int> textures;

bool updatedMovement = true;

static bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

unsigned int loadTexture(std::string path)
{
	unsigned int tex;

	int width, height, nrChannels;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
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

			if (endsWith(str, ".png"))
				continue;

			if (count++ >= 100)
				break;

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

void moveCamera(int x, int y, float delta)
{
	float newX = camera.posx + (x * cameraSpeed * delta);
	float newY = camera.posy + (y * cameraSpeed * delta);

	if (newX >= 0 && newY >= 0 && newX <= dungeon->getMaxDimension() && newY <= dungeon->getMaxDimension())
	{
		camera.posx = newX;
		camera.posy = newY;

		updatedMovement = true;
	}
}

void processInput(GLFWwindow *window, float delta)
{
	// ESCAPE!!
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		moveCamera(-1, 0, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		moveCamera(1, 0, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		moveCamera(0, -1, delta);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		moveCamera(0, 1, delta);
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

	glfwWindowHint(GL_MULTISAMPLE, 4);

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// nice dull blue colour
	glClearColor(0.4f, 0.0f, 1.0f, 1.0f);
	
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

	return 1;
}

glm::mat4 calcTransform(Tile t)
{	
	glm::mat4 mat = glm::scale(glm::mat4(1), scale); 
	return glm::translate(mat, glm::vec3((t.posX * TILE_SIZE) - camera.posx, (t.posY * TILE_SIZE) - camera.posy, 0.0f));	 
}

std::vector<Tile> visibleTiles;
void render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	shader->use();

	//shader->setUniform("projection", Projection);

	glBindVertexArray(VAO);

	
	if (updatedMovement)
		visibleTiles = dungeon->getVisibleTiles(camera.posx, camera.posy);
	

	for (Tile t : visibleTiles)
	{
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, t.id);

		shader->setUniform("transform", calcTransform(t));
	//	shader->setUniform("colour", t.colour);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	updatedMovement = false;

	glBindVertexArray(0);

	shader->disable();
}

void update(float delta)
{

}

void gameLoop()
{
#ifdef DEBUG_ON	
	printf("Entering game loop...\n");
#endif

	float currentFrame;

	double lag = 0.0;
	
#ifdef DEBUG_ON	
	double time = 0.0;
	int frames = 0;
#endif
	
	while (!glfwWindowShouldClose(window))
	{
		currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		lag += deltaTime;

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
		
		while (lag >= TARGET_UPDATE_TIME)
		{
			update(lag / TARGET_UPDATE_TIME);
			lag -= TARGET_UPDATE_TIME;
		}

		processInput(window, deltaTime / TARGET_UPDATE_TIME);

		render();

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
	
	camera = { 0.0f, 0.0f };

	gameLoop();

	delete dungeon;
	delete shader;

	glfwTerminate();
    return 0;
}

