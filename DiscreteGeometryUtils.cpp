#include "DiscreteGeometryUtils.h"

std::vector<ImplicitGeo::Triangle> DiscreteGeometryUtils::getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object)
{
	auto vertexBuffer = (Graphics::MeshObject*)object->signatureLookup("VERTEX");
	auto vertices = vertexBuffer->vertices;
	auto indices = vertexBuffer->indices;
	auto transformBuffer = (Graphics::MatrixInstancedMeshObject<mat4, float>*)object->signatureLookup("TRANSFORM");
	auto transforms = transformBuffer->extendedData;

	vector<ImplicitGeo::Triangle> triangles;

	for (int i = 0; i < transforms.size(); i++)
	{
		mat4 t = transforms[i];
		vector<vec3> transformedPts;

		for (int j = 0; j < vertices.size(); j++)
		{
			transformedPts.push_back(vec3(t * vec4(vertices[j].position, 1)));
		}

		for (int j = 0; j < indices.size(); j += 3)
		{
			triangles.push_back(ImplicitGeo::Triangle(transformedPts[indices[j]], transformedPts[indices[j + 1]], transformedPts[indices[j + 2]]));
		}
	}

	return triangles;
}

std::pair<glm::vec3, glm::vec3> DiscreteGeometryUtils::getBoundingBox(Graphics::DecoratedGraphicsObject* object)
{
	auto vertexBuffer = (Graphics::MeshObject*)object->signatureLookup("VERTEX");
	auto vertices = vertexBuffer->vertices;
	auto transformBuffer = (Graphics::MatrixInstancedMeshObject<mat4, float>*)object->signatureLookup("TRANSFORM");
	auto transforms = transformBuffer->extendedData;

	vec3 minN(INFINITY);
	vec3 maxN(-INFINITY);

	for (int i = 0; i < transforms.size(); i++)
	{
		mat4 t = transforms[i];
		vector<vec3> transformedPts;

		for (int j = 0; j < vertices.size(); j++)
		{
			transformedPts.push_back(vec3(t * vec4(vertices[j].position, 1)));

			for (int k = 0; k < 3; k++)
			{
				if (transformedPts[transformedPts.size() - 1][k] < minN[k])
				{
					minN[k] = transformedPts[transformedPts.size() - 1][k];
				}
				else if (transformedPts[transformedPts.size() - 1][k] > maxN[k])
				{
					maxN[k] = transformedPts[transformedPts.size() - 1][k];
				}
			}
		}
	}

	return std::make_pair(minN, maxN);
}

bool DiscreteGeometryUtils::isPointInsideMesh(glm::vec3 point, const std::vector<ImplicitGeo::Triangle>& triangles)
{
	vec3 dir(1, 0, 0);
	int sum = 0;
	for (int j = 0; j < triangles.size(); j++)
	{
		if (triangles[j].intersection(point, dir) > 0.0f)
		{
			sum++;
		}
	}

	return sum % 2 == 1;
}