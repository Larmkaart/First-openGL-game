#ifndef NOISE_H
#define NOISE_H
#include "External/stb_perlin.h"

class Noise
{
public:
	static float* GenerateNoiseMap(int mapWidth, int mapHeight, vec2 offset, int seed, float scale, int octaves, float persistance, float lacunarity)
	{
		float* noiseMap = new float [mapWidth * mapHeight];

		srand(seed);

		float amplitude = 1.0f;
		float frequency = 1.0f;
		float maxHeight = 0.0f;

		vector<vec2> octaveOffsets = vector<vec2>(octaves);
		for (int i = 0; i < octaves; i++)
		{
			float offsetX = (rand() % 10000 - 5000) + offset.x;
			float offsetY = (rand() % 10000 - 5000) + offset.y;
			octaveOffsets[i] = vec2(offsetX, offsetY);

			maxHeight += amplitude;
			amplitude *= persistance;
		}
	

		if (scale <= 0)
		{
			scale = 0.0001f;
		}

		float minValue = std::numeric_limits<float>::max();
		float maxValue = std::numeric_limits<float>::min();

		float halfWidth = mapWidth / 2.0f; // Make sure the map zooms into the centre
		float halfHeight = mapHeight / 2.0f;


		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{

				amplitude = 1.0f;
				frequency = 1.0f;

				float result = 0.0f;
				float influence = 0.0f;

				for (int octave = 0; octave < octaves; octave++)
				{
					float sampleX = (x + octaveOffsets[octave].x)  / scale * frequency;
					float sampleY = (y + octaveOffsets[octave].y) / scale * frequency;

					float perlinValue = stb_perlin_noise3(sampleX, 0, sampleY, 0, 0, 0);

					
					float h = 0.001;
					float dX = stb_perlin_noise3(sampleX + h, 0, sampleY, 0, 0, 0) - perlinValue;
					float dY = stb_perlin_noise3(sampleX, 0, sampleY + h, 0, 0, 0) - perlinValue;

					dX /= h;
					dY /= h;

					vec2 gradient = vec2(dX, dY);
					influence += 1 / (1 + length(gradient) * 2);
					

					result += perlinValue * amplitude * influence;

					amplitude *= persistance;
					frequency *= lacunarity;
				}

				if (result > maxValue)
					maxValue = result;
				else if (result < minValue)
					minValue = result;

				noiseMap[x + y * mapWidth] = result;
			}
		}

		/*
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				noiseMap[x + y * mapWidth] = noiseMap[x + y * mapWidth] / (maxHeight / 2.0f); // (noiseMap[x + y * mapWidth] + 1.0f) / (2.0f * maxHeight);
			}
		}
		*/
		return noiseMap;
	}

	static float LevelOfDetail(float value)
	{
		return value * value;
		//return 0.6f * value * value * value * value + 0.4f * value;
	}
};

#endif