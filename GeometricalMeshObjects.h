#pragma once

#include "GraphicsObject.h"

class Quad : public MeshObject
{
public:
	Quad();
};

class Polyhedron : public MeshObject
{
public:
	int resolution;
	vec3 radii;
	Polyhedron(int resolution, vec3 pos, vec3 radii);
};

class Tetrahedron : public MeshObject
{
public:
	Tetrahedron();
};