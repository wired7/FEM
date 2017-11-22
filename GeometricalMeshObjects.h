#pragma once

#include "GraphicsObject.h"

class Quad : public MeshObject
{
public:
	Quad();
};

class Cylinder : public MeshObject
{
public:
	Cylinder(int resolution);
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