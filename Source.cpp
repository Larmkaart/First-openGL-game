#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>

/* In order to make sure GLFW doesn't define a development header, define GLFW_INCLUDE_NONE before including GLFW */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "World.h"
#include "Noise.h"
#include "Player.h"
#include "Physics.h"

#include "External/stb_image.h"
#include "External/stb_image_write.h"


using namespace std;
using namespace glm;

unsigned int width = 800;
unsigned int height = 600;

const float TIME_STEP = 1 / 60.0f; // 60fps

int chunkRenderDistance = 15;
int maxChunksPerFrame = 5;
bool firstRender = true;
list<ivec2> chunkQueue;

void processInput(GLFWwindow* window);
void error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void generateTexture(unsigned int* target, const char* filePath, GLenum textureDepth = GL_TEXTURE0, GLenum wrapMode = GL_REPEAT, GLenum minFiter = GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter = GL_LINEAR);


Camera camera(vec3(0.0f, 0.0f, 3.0f));
//Player player(camera);
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
	vec3 a = vec3(-50, 0, 0);
	vec3 b = vec3(157, -23, 0);
	vec3 c = vec3(74, 138, 0);
	Sphere sphere;
	sphere.r = 35;
	sphere.position = vec3(140, 60, 0);
	vec3 point = vec3(0);
	int result = Physics::TestSphereTriangle(sphere, a, b, c, point);

	std::cout << result;


	return 1;
	if (!glfwInit())
	{
		std::cout << "Failed to initialise GLFW" << std::endl;
		return -1;
	}

	/* We are using openGL 3.3 core, the core profile removes deprecated functions */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

	GLFWwindow* window = glfwCreateWindow(width, height, "Window", NULL, NULL);

	if (!window)
	{
		std::cout << "Failed to create window context" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetErrorCallback(error_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	 /* Here we make sure glad can access the openGL functions through glfw */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to load glad" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	stbi_set_flip_vertically_on_load(true);

	/* Here we set up our shader*/
	Shader shader("Shaders\\Shader.vert", "Shaders\\Shader.frag");
	shader.use();

	World world;

	PhysicsEngine engine;


	unsigned int noiseMap;
	glGenTextures(1, &noiseMap);
	generateTexture(&noiseMap, "Textures/grass.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE, noiseMap);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		engine.update(deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		mat4 projection = perspective(radians(camera.Zoom), static_cast<float>(width / height), 0.1f, 1000.0f);
		mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		mat4 model = mat4(1.0f);
		model = translate(model, vec3(0.0f));
		model = scale(model, vec3(1.0f));
		shader.setMat4("model", model);

		shader.setVec3("viewPos", camera.Position);

		ivec2 cameraChunk = vec2(
			round((camera.Position.x / world.vertexScale) / world.ChunkSize),
			round((camera.Position.z / world.vertexScale) / world.ChunkSize)
			);

		glActiveTexture(GL_TEXTURE0);

		for (int y = -chunkRenderDistance; y <= chunkRenderDistance; y++)
		{
			for (int x = -chunkRenderDistance; x <= chunkRenderDistance; x++)
			{
				int chunkX = x + cameraChunk.x;
				int chunkY = y + cameraChunk.y;

				// Look up the chunk
				Chunk* currentChunk = world.Chunks[Chunk::chunkHash(chunkX, chunkY)];
				
				// Check if the chunk is preloaded
				if (currentChunk == nullptr)
				{
					// Make sure first render frame starts with generating all the chunks
					if (firstRender)
					{
						currentChunk = new Chunk(world, ivec2(chunkX, chunkY)); // If the chunk doesn't exist, generate it
						world.Chunks[currentChunk->chunkID] = currentChunk;
						continue;
					}

					bool skip = false;
					// If not, push it unto the queue
					for (ivec2 pos : chunkQueue)
					{
						if (pos == ivec2(chunkX, chunkY))
						{
							skip = true;
							break;
						}
					}

					if (!skip)
						chunkQueue.push_back(ivec2(chunkX, chunkY));

					continue;
				}

				// If the chunk is loaded, draw it
				currentChunk->surfaceMesh.Draw(shader);

				//Chunk::saveChunk(currentChunk); Save the chunk to a file (still too laggy to be useful)
			}
		}

		firstRender = false;
		bool didRender = false;

		// Create chunks from the chunk queue
		for (int i = 0; i < maxChunksPerFrame; i++)
		{
			if (chunkQueue.size() < 1) break;
			didRender = true;
			ivec2 chunkPos = chunkQueue.front();
			chunkQueue.pop_front();

			Chunk* currentChunk = new Chunk(world, chunkPos);
			world.Chunks[currentChunk->chunkID] = currentChunk;

			//std::cout << currentChunk->chunkID << std::endl;
		}

		if (didRender)
		{
			// Some optimisation to prevent low FPS
			if (1 / deltaTime < 30 && maxChunksPerFrame > 1)
				maxChunksPerFrame--;
			else if (1 / deltaTime > 60)
				maxChunksPerFrame++;

			// Make sure it won't fall behind
			if (chunkQueue.size() / maxChunksPerFrame > chunkRenderDistance * 8)
			{
				maxChunksPerFrame += chunkRenderDistance;
			}

			//std::cout << maxChunksPerFrame << "\t" << 1 / deltaTime << std::endl;
		}
;		
		/* GLFW works using 2 buffers: a display buffer and a hidden buffer. This is to prevent screen tearing */
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(LOOK_UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(LOOK_DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(LOOK_LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(LOOK_RIGHT, deltaTime);
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
	glViewport(0, 0, _width, _height);
	width = _width;
	height = _height;
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) // initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void generateTexture(unsigned int* target, const char* filePath, GLenum textureDepth, GLenum wrapMode, GLenum minFiter, GLenum magFilter)
{
	// Initialise texture
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// Prepare necessary texture data

	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else
		{
			format = GL_RGB;
			std::cout << "color format '" << nrChannels << "' not found, continuing with GL_RGB\n";
		}

		glActiveTexture(textureDepth);
		glBindTexture(GL_TEXTURE_2D, textureID);
		// Now that the texture reference is set up, we can start generating a texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);


		std::cout << "Loaded texture: '" << filePath << "'!\n";
	}
	else
	{
		std::cout << "Failed to load texture: '" << filePath << "'\n";
	}
	stbi_image_free(data);

	*target = textureID;
}