#ifndef PLAYER_H
#define PLAYER_H

class Player
{
public:
	vec3 Position;
	vec3 Orientation;
	Camera camera;
	Sphere hitBox;
	vector<Mesh> playerColliders;

	Player(Camera _camera, vec3 _position = vec3(0), vec3 _orientation = vec3(1.0f, 0.0f, 0.0f)) : camera(_camera), Position(_position), Orientation(_orientation)
	{
		hitBox.position = _position;
		hitBox.r = 2;
		//engine = PhysicsEngine();
	}

	void ProcessMovement()
	{

	}

	void AddCollider(Mesh mesh) { playerColliders.push_back(mesh); }

	void ClearColliders() { playerColliders.clear(); }

private:
	PhysicsEngine engine;
	void processCollision()
	{
		ivec2 chunkPos = { 
			round((camera.Position.x / world.vertexScale) / world.ChunkSize),
			round((camera.Position.z / world.vertexScale) / world.ChunkSize) 
		}
	}
	void collideWithMesh(Mesh mesh)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				vec3* triangle = mesh.GetTriangle(x + y * 32);
				vec3 pointOnTriangle = vec3(0);
				int didHit = Physics::TestSphereTriangle(hitBox, triangle[0], triangle[1], triangle[2], pointOnTriangle);
				std::cout << didHit << std::endl;
			}
		}
	}
};

#endif