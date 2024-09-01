#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include "Noise.h"
#include <vector>
#include <string>

class MapGenerator
{
public:
	int mapWidth;
	int mapHeight;
	float noiseScale;

	int octaves;
	float persistance;
	float lacunarity;

	int seed;

	void GenerateMap()
	{
		if (mapWidth < 1)
			mapWidth = 1;
		if (mapHeight < 1)
			mapHeight = 1;

		if (octaves < 0)
			octaves = 0;

		if (lacunarity < 1)
			lacunarity = 1.0f;
		if (persistance < 0 || persistance > 1)
			persistance = 0.5f;

		float* noiseMap = Noise::GenerateNoiseMap(mapWidth, mapHeight, seed, noiseScale, octaves, persistance, lacunarity);


	}
};

#endif