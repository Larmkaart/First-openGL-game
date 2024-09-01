#ifndef PHYSICS_H
#define PHYSICS_H

class Object
{
public:
	vec3 position;
	vec3 velocity;
	vec3 acceleration;
	float mass;

	Object(vec3 pos, float m) : position(pos), mass(m) {}

	void update(float deltaTime)
	{
		velocity += acceleration * deltaTime;
		position += velocity * deltaTime;
		acceleration = vec3(0);
	}
};

class PhysicsEngine
{
public:
	vector<Object> objects;

	void addObject(const Object& object)
	{
		objects.push_back(object);
	}
		
	void update(float deltaTime)
	{
		for (auto& obj : objects)
		{
			obj.update(deltaTime);
		}
	}
};


static class Physics
{
public:
	// See file:///C:/Users/Lenovo/Downloads/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf P. 179 for possible optimisation
	static vec3 ClosestPtPointTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
	{
		// Calculate line segments of the triangle
		vec3 ab = b - a;
		vec3 ac = c - a;
		vec3 bc = c - b;

		// Compute parametric position s for projection P' of P on line AB
		// P' = A + s*AB
		// s is the barycentric coordinate along the AB segment (range 0-1),
		// which can be defind as:
		// s = snom/(snom + sdenom)
		float snom = dot(p - a, ab); // project P onto AB, relative to A
		float sdenom = dot(p - b, a - b); // project P onto BA, relative to B

		// Compute parametric position t for projection P' of P on AC
		float tnom = dot(p - a, ac);
		float tdenom = dot(p - c, a - c);

		// if snom and tnom <= 0, P' lies in the vonoroi region of point A, so point A is the closest point
		if (snom <= 0.0f && tnom <= 0.0f) return a;

		// Compute parametric position u for projection P' of P on BC
		float unom = dot(p - b, bc);
		float udenom = dot(p - c, b - c);

		// if sdenom and unom <= 0, P' lies in the vonorio region of point B, so point B is the closest point
		if (sdenom <= 0.0f && unom <= 0.0f) return b;
		// if tdenom and undeom <= 0, P' lies in the vonorio region of point C, so point C is the closest point
		if (tdenom <= 0.0f && udenom <= 0.0f) return c;

		// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
		vec3 normal = cross(b - a, c - a);
		float vc = dot(normal, cross(a - p, b - p)); // triple scalar product (calculates the signed volume of the parallelepiped between N, PA and PB)
		// if P is ouside AB but within the feature region of AB, (aka within the plane made by the two vertices)
		// return the projection P' of P onto AB
		if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f) // snom >= 0.0f && sdenom >= 0.0f means point P' lies bewteen A and B
			return a + snom / (snom + sdenom) * ab;

		// P is outside (or on) BC if the triple scalar product [N, PB, PC] <= 0
		float va = dot(normal, cross(b - p, c - p));
		// if P is outside BC but within the feature region of BC,
		// return projection P' of P onto BC
		if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
			return b + unom / (unom + udenom) * bc;

		// same for CA
		float vb = dot(normal, cross(c - p, a - p));
		if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
			return a + tnom / (tnom + tdenom) * ac;

		// P' must be inside the face region, Compute point Q usin barycentric coordinates
		// barycentric coordinates are calculated using the ratio between the triangles abp + acp + bcp = abc
		// in this case va, vb, and vc represent the volume of the parallelepiped with the normal, but the idea should stay the same
		float u = va / (va + vb + vc);
		float v = vb / (va + vb + vc);
		float w = 1.0f - u - v; // w = vc / (va + vb + vc) could also be used, but this saves us a division :P
		// this works because u + v + w = 1.0f, otherwise it could not describe a triangle
		// (1, 0, 0) = A, (0, 1, 0) = B, (0, 0, 1) = C
		// increasing any component would put the point outside of the triangle!

		return u * a + v * b + w * c;
	}

	static int TestSphereTriangle(Sphere sphere, vec3 A, vec3 B, vec3 C, vec3& P)
	{
		P = ClosestPtPointTriangle(sphere.position, A, B, C);

		vec3 v = P - sphere.position;
		return dot(v, v) < sphere.r * sphere.r;
		// dot(v, v) = ||v||²
	}
};



#endif