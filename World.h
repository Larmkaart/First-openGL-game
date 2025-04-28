#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <map>

#include "Noise.h"
#include "Renderer.h"

using namespace std;
using namespace glm;

class Chunk;

class World
{
public:
	int ChunkSize;
	const int seed = time(NULL);
	int octaves;
	float noiseScale;
	float persistance;
	float lacunarity;
	float heightScale;
	float vertexScale;
	float influenceScale;

	float globalMinimum;
	float globalMaximum;

	map<double, Chunk*> Chunks;

	World()
	{
		noiseScale = 75.0f;
		octaves = 8;
		persistance = 0.2f; //(0-1) controls how much the amplitude changes
		lacunarity = 4.0f; // controls how much the frequency changes
		influenceScale = 0.0f;
		heightScale = 30.0f;
		ChunkSize = 32;
		vertexScale = 5.0f;

		globalMinimum = std::numeric_limits<float>::max();
		globalMaximum = std::numeric_limits<float>::min();

		std::cout << "Started world using seed: " << this->seed << std::endl;

		if (ChunkSize % 2 != 0)
			ChunkSize++; // Ensure ChunkSize is even
	}

};

class Chunk
{
public:
	double chunkID;
	Mesh surfaceMesh;
	ivec2 position;
	Texture texture;
	bool isQueued = false;

	Chunk(World* world, ivec2 chunkPosition)
	{
		position = chunkPosition;
		chunkID = chunkHash(position.x, position.y);

		generateChunk(world, chunkPosition);
	}

	Chunk() {}

	static Chunk* loadChunk(World world, double chunkID)
	{
		Chunk chunk;
		string path = "Chunks\\" + to_string(chunkID) + ".chunk";
		ifstream file(path, ios::binary);
		if (file.is_open())
		{
			size_t size;
			file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
			
			chunk.surfaceMesh.vertices.resize(size);
			file.read(reinterpret_cast<char*>(chunk.surfaceMesh.vertices.data()), size * sizeof(Vertex));

			file.read(reinterpret_cast<char*>(&size), sizeof(size_t));

			chunk.surfaceMesh.indices.resize(size);
			file.read(reinterpret_cast<char*>(chunk.surfaceMesh.indices.data()), size * sizeof(unsigned int));
		}
	}

	static void saveChunk(Chunk* chunk) 
	{
		string path = "Chunks\\" + to_string(chunk->chunkID) + ".chunk";
		ofstream file(path, ios::binary);
		if (file.is_open())
		{
			size_t size = chunk->surfaceMesh.vertices.size();
			file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

			file.write(reinterpret_cast<const char*>(chunk->surfaceMesh.vertices.data()), size * sizeof(Vertex));

			size = chunk->surfaceMesh.indices.size();
			file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

			file.write(reinterpret_cast<const char*>(chunk->surfaceMesh.indices.data()), size * sizeof(unsigned int));
			file.close();
		}
	}

	void generateChunk(World* world, ivec2 chunkPosition)
	{
		int chunkSize = world->ChunkSize;
		int vertexChunkSize = chunkSize + 1;
		Noise::NoiseMap2D* noiseData = Noise::GenerateNoiseMap(vertexChunkSize, vertexChunkSize, chunkPosition * chunkSize, world->seed, world->noiseScale,  world->octaves, world->persistance, world->lacunarity, world->influenceScale);

		if (noiseData->max > world->globalMaximum)
			world->globalMaximum = noiseData->max;
		if (noiseData->min < world->globalMinimum)
			world->globalMinimum = noiseData->min;

		surfaceMesh.drawMode = INDEX;

		vector<Vertex> vertices;
		vector<unsigned int> indices;

		vec3 chunkStartPos = vec3(chunkPosition.x * chunkSize - chunkSize * 0.5f, 0.0f, chunkPosition.y * chunkSize - chunkSize * 0.5f);

		for (int y = 0; y < chunkSize; y++)
		{
			for (int x = 0; x < chunkSize; x++)
			{
				double vertexBase = (x + y * chunkSize) * 4;
				double indexBase = (x + y * chunkSize) * 6;

				float noise0 = noiseData->map[x + y * vertexChunkSize];// *world->heightScale;
				float noise1 = noiseData->map[x + (y + 1) * vertexChunkSize];// *world->heightScale;
				float noise2 = noiseData->map[(x + 1) + (y + 1) * vertexChunkSize];// *world->heightScale;
				float noise3 = noiseData->map[(x + 1) + y * vertexChunkSize];// *world->heightScale;

				vec3 A = vec3(x, noise0, y); // Bottom left
				vec3 B = vec3(x, noise1, y + 1); // Top left
				vec3 C = vec3(x + 1, noise2, y + 1); // Top right
				vec3 D = vec3(x + 1, noise3, y); // Bottom right

				// Calculate avarages on line
				vec3 AB = (A + B) / 2.0f;
				vec3 AD = (A + D) / 2.0f;

				// Calculate mid-point
				vec3 S = (A + B + C + D) / 4.0f;

				// Calculate vectors pointing from middle
				vec3 E = normalize(S - AB);
				vec3 F = normalize(S - AD);
				
				// Calculate new vector using cross product
				vec3 normal = cross(F, E);   //  normalize(cross(normalize(E), normalize(F)));

				//std::cout << "(" << normal.x << ", " << normal.y << ", " << normal.z << ")\n";


				Vertex vertex;

				// Bottom left corner (first triangle)
				vertex.Position = (vec3(x, noise0, y) + chunkStartPos) * world->vertexScale;
				vertex.TexCoords = vec2(-1.0f, -1.0f);
				vertex.Normal = normal;
				vertices.push_back(vertex);
				indices.push_back(vertexBase);

				// Top Left corner (first triangle)
				vertex.Position = (vec3(x, noise1, (y + 1)) + chunkStartPos) * world->vertexScale;
				vertex.TexCoords = vec2(1.0f, -1.0f);
				vertex.Normal = normal;

				vertices.push_back(vertex);
				indices.push_back(vertexBase + 1);

				// Top right corner (first triangle)
				vertex.Position = (vec3((x + 1), noise2, (y + 1)) + chunkStartPos) * world->vertexScale;
				vertex.TexCoords = vec2(1.0f, 1.0f);
				vertex.Normal = normal;

				vertices.push_back(vertex);
				indices.push_back(vertexBase + 2);

				// Bottom left corner (second triangle)
				indices.push_back(indices[indexBase]);

				// Top right corner (second triangle)
				indices.push_back(indices[indexBase + 2]);

				// Bottom right corner (second triangle)
				vertex.Position = (vec3((x + 1), noise3, y) + chunkStartPos) * world->vertexScale;
				vertex.TexCoords = vec2(-1.0f, 1.0f);
				vertex.Normal = normal;


				vertices.push_back(vertex);
				indices.push_back(vertexBase + 3);
			}
		}

		surfaceMesh.DumpVertices(vertices);
		surfaceMesh.DumpIndices(indices);

		surfaceMesh.Start();

		delete(noiseData->map);
	}

	static double chunkHash(int x, int y)
	{
		// Convert x and y components to integers for hashing
		int x_hash = x * 1000; // Scale as needed
		int y_hash = y * 1000; // Scale as needed

		// Combine x and y hashes into a single hash value
		return x_hash * 73856093 + y_hash * 19349663; // Large prime numbers for mixing
	}
};

#endif