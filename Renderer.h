#ifndef RENDERER_H
#define RENDERER_H


using namespace std;
using namespace glm;


struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

struct Sphere
{
	vec3 position;
	float r;
};

struct Texture
{
	unsigned int id;
	string path;
	/* The texture may have a special function like Specular or Normal maps*/
	string type;
};

enum DrawMode { VERTEX, INDEX };

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	/* 
	Set the draw mode of the vertices 
		VERTEX: Exclusively use vertex data
		INDEX: Use index data
	*/
	DrawMode drawMode;

	void Draw(Shader &shader)
	{
		glBindVertexArray(VAO);
		
		if (drawMode == VERTEX) 
		{
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(Vertex));
		}
		else if (drawMode == INDEX) 
		{
			glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
	}

	vec3* GetTriangle(int index)
	{
		vec3 triangle[3] = {vec3(0), vec3(0), vec3(0)};

		if (index % 2 == 0)
		{
			triangle[0] = vertices[indices[index * 4 + 0]].Position;
			triangle[1] = vertices[indices[index * 4 + 1]].Position;
			triangle[2] = vertices[indices[index * 4 + 2]].Position;
			return triangle;
		} else
		{
			triangle[0] = vertices[indices[index * 4 + 0]].Position;
			triangle[1] = vertices[indices[index * 4 + 2]].Position;
			triangle[2] = vertices[indices[index * 4 + 3]].Position;
			return triangle;
		}
	}

	/* Dump an array containing all the mesh data with format (px, py, pz, nx, ny ,nz, u, v) */
	void DumpVertexArray(float* arr, unsigned int size)
	{
		for (unsigned int i = 0; i < size; i += 8)
		{
			float* data = arr + i;

			Vertex vert;

			vert.Position  = vec3(*(data + 0), *(data + 1), *(data + 2));
			vert.Normal    = vec3(*(data + 3), *(data + 4), *(data + 5));
			vert.TexCoords = vec2(*(data + 6), *(data + 7));

			vertices.push_back(vert);
		}
	}

	void DumpIndexArray(unsigned int* arr, unsigned int size)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			indices.push_back(*(arr + i));
		}
	}

	void DumpIndices(vector<unsigned int> arr)
	{
		for (unsigned int i = 0; i < arr.size(); i++)
		{
			indices.push_back(arr[i]);
			//std::cout << arr[i] << ", ";
		}
	}

	void DumpVertices(vector<Vertex> arr)
	{
		for (unsigned int i = 0; i < arr.size(); i++)
		{
			vertices.push_back(arr[i]);
			//vec3 pos = arr[i].Position;
			//std::cout << "( " << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
		}
	}

	void Start()
	{
		setupMesh();
	}
private:
	unsigned int VAO, VBO, EBO;

	void unbindMesh()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void setupMesh()
	{
		/* Generate buffers */
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		/* 
		Set up the VAO
		We will first assign a VBO to the VAO containing the vertex data. This data wll be used for the rendering process.
		*/

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		/* If we don't want to use indices, we can save some space by not assigning memory to it */
		if (drawMode == INDEX)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		}

		/* Assign position data */
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		/* Assign normal data */
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(1);
		/* Assign texture data */
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};


#endif