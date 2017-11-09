#pragma once

#include "GeometricalMeshObjects.h"
#include <iostream>

Quad::Quad() : MeshObject()
{
	addVertex(vec3(-1, -1, 0), vec3());
	addVertex(vec3(1, -1, 0), vec3());
	addVertex(vec3(-1, 1, 0), vec3());
	addVertex(vec3(1, 1, 0), vec3());

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);

	bindBuffers();
}

Polyhedron::Polyhedron(int resolution, vec3 pos, vec3 radii) : MeshObject()
{
	model = scale(translate(mat4(1.0f), pos), radii);
	this->resolution = resolution;

	double theta = 2 * 3.1415 / resolution;
	double phi = 3.1415 / resolution;
	vector<vector<vec3>> circles;
	vec3 start(0, -0.5f, 0);
	for (int j = 1; j < resolution; j++)
	{
		vec3 temp = rotate(start, (GLfloat)(phi * j), vec3(0, 0, 1));
		circles.push_back(vector<vec3>());
		for (int i = 0; i < resolution; i++)
			circles[circles.size() - 1].push_back(rotate(temp, (GLfloat)(theta * i), vec3(0, 1, 0)));
	}

	for (int j = 0; j < circles.size(); j++)
	{
		for (int i = 0; i < circles[j].size(); i++)
		{
			addVertex(circles[j][i], normalize(circles[j][i]));

			if (j > 0 && i > 0)
			{
				indices.push_back(vertices.size() - 2 - circles[j].size());
				indices.push_back(vertices.size() - 2);
				indices.push_back(vertices.size() - 1 - circles[j].size());

				indices.push_back(vertices.size() - 2);
				indices.push_back(vertices.size() - 1);
				indices.push_back(vertices.size() - 1 - circles[j].size());
			}
		}

		if (j > 0)
		{
			indices.push_back(vertices.size() - 1 - circles[j].size());
			indices.push_back(vertices.size() - 1);
			indices.push_back(vertices.size() - 2 * circles[j].size());

			indices.push_back(vertices.size() - 1);
			indices.push_back(vertices.size() - 2 * circles[j].size());
			indices.push_back(vertices.size() - circles[j].size());
		}
	}

	addVertex(start, start);
	addVertex(-start, -start);

	for (int i = 0; i < resolution; i++)
	{
		indices.push_back(i);
		indices.push_back((i + 1) % resolution);
		indices.push_back(vertices.size() - 2);

		indices.push_back(vertices.size() - 2 - resolution + i);
		indices.push_back(vertices.size() - resolution + (i + 1) % resolution - 2);
		indices.push_back(vertices.size() - 1);
	}

	bindBuffers();
}

Tetrahedron::Tetrahedron() : MeshObject()
{
	model = scale(translate(mat4(1.0f), vec3()), vec3(1, 1, 1));
	addVertex(vec3(-1, 0, 0), vec3());
	addVertex(vec3(1, 0, 0), vec3());
	addVertex(vec3(0, 1, 0), vec3());
	addVertex(vec3(0, 0, 1), vec3());
	
	vec3 centroid(0, 0, 0);
	for (int i = 0; i < vertices.size(); i++)
	{
		centroid += vertices[i].position;
	}

	centroid /= vertices.size();

	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].normal = normalize(vertices[i].position - centroid);
	}

	indices = { 0, 3, 1, 0, 2, 1, 0, 3, 2, 1, 2, 3};

	bindBuffers();
}
