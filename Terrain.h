#pragma once

class Terrain
{
public:
	struct TerrainTypes
	{
	public: 
		const char* name;
		float height;
		vec4 color;

		TerrainTypes(const char* name, float height, vec4 color) {
			this->name = name;
			this->height = height;
			this->color = color;
		}
	};

	TerrainTypes* regions[2] = {
		new TerrainTypes("water", 0.4f, vec4(70.0f, 115.0f, 200.0f, 1.0f)),
		new TerrainTypes("land", 1.0f, vec4(85.0f, 150.0f, 20.0f, 1.0f))
	};

private:
};