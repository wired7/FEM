#pragma once

#include "GraphicsObject.h"

namespace Graphics
{
	class Triangle : public Graphics::MeshObject
	{
	public:
		Triangle();
	};

	class Quad : public Graphics::MeshObject
	{
	public:
		Quad();
	};

	class Cylinder : public Graphics::MeshObject
	{
	public:
		Cylinder(int resolution);
	};

	class Polyhedron : public Graphics::MeshObject
	{
	public:
		int resolution;
		vec3 radii;
		Polyhedron(int resolution, vec3 pos, vec3 radii);
	};

	class Tetrahedron : public Graphics::MeshObject
	{
	public:
		Tetrahedron();
	};

	class Arrow : public Graphics::MeshObject
	{
	public:
		Arrow();
	};
}