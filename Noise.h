#ifndef NOISE_H
#define NOISE_H
#include "External/stb_perlin.h"

class Noise
{
public:
	struct NoiseMap2D
	{
		float* map;
		
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::min();

		NoiseMap2D(int width, int height)
		{
			map = new float[width * height];
		}
	};


	static NoiseMap2D* GenerateNoiseMap(int mapWidth, int mapHeight, vec2 offset, int seed, float scale, int octaves, float persistance, float lacunarity, float influenceScale)
	{
		NoiseMap2D* noiseMap = new NoiseMap2D(mapWidth, mapHeight);

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
				float weight = 1.0f;

				for (int octave = 0; octave < octaves; octave++)
				{
					float sampleX = (x + octaveOffsets[octave].x)  / scale * frequency;
					float sampleY = (y + octaveOffsets[octave].y) / scale * frequency;

					float perlinValue = 1 + stb_perlin_noise3(sampleX, 0, sampleY, 0, 0, 0) / 2;

					perlinValue *= perlinValue;
					perlinValue *= weight;
					weight = perlinValue;
					
					// Ensure the influence of each octave decreases 
					result += perlinValue * amplitude * 20.0f;

					amplitude *= persistance;
					frequency *= lacunarity;
				}

				if (result > noiseMap->max)
					noiseMap->max = result;
				if (result < noiseMap->min)
					noiseMap->min = result;

				noiseMap->map[x + y * mapWidth] = result;
				//std::cout << noiseMap->map[x + y * mapWidth] << std::endl;
			}
		}

		//std::cout << "Min: " << noiseMap->min << " Max: " << noiseMap->max << std::endl;

		return noiseMap;
	}

	static float LevelOfDetail(float value)
	{
		return value;// abs(value * value);// *(value / abs(value));
		//return 0.6f * value * value * value * value + 0.4f * value;
	}
};

#endif