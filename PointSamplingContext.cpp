#pragma once
#include "PointSamplingContext.h"
#include "ImplicitGeometry.h"
#include <thread>

using namespace std;
using namespace Graphics;

#define NUM_POINTS 1000
#define POINT_SCALE 0.50f
PointSamplingContext::PointSamplingContext(DecoratedGraphicsObject* surface, SphericalCamera* cam)
{
	cameras.push_back(cam);
	geometries.push_back(surface);
	setupPasses();

	thread t([&] {
		points = sampleSurface(NUM_POINTS, surface);
		pointsReady = true;
	});
	t.detach();
}

void PointSamplingContext::setupGeometries(void)
{
	((GeometryPass*)passRootNode)->clearRenderableObjects(1);

	auto m = new Polyhedron(10, vec3(), vec3(1.0f));

	vector<mat4> transform;

	for (int i = 0; i < points.size(); i++)
	{
		transform.push_back(translate(mat4(1.0f), points[i]) * scale(mat4(1.0f), glm::vec3(1,1,1)*POINT_SCALE));
	}

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, transform, "OFFSET");

	vector<GLuint> instanceID;

	for (int i = 1; i <= transform.size(); i++)
	{
		instanceID.push_back(i);
	}

	auto pickable = new InstancedMeshObject<GLuint, GLuint>(g, instanceID, "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, 1);
	dirty = true;
}

void PointSamplingContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A"), ShaderProgramPipeline::getPipeline("C") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void PointSamplingContext::update(void)
{
	GraphicsSceneContext<PointSamplingController, SphericalCamera, PointSamplingContext>::update();

	if (pointsReady)
	{
		setupGeometries();
		pointsReady = false;
		readyToAdvance = true;
	}
}

vector<vec3> PointSamplingContext::sampleSurface(int sampleSize, DecoratedGraphicsObject* object)
{
	vector<vec3> samples;

	auto vertexBuffer = (MeshObject*)object->signatureLookup("VERTEX");
	auto vertices = vertexBuffer->vertices;
	auto indices = vertexBuffer->indices;
	auto transformBuffer = (MatrixInstancedMeshObject<mat4, float>*)object->signatureLookup("TRANSFORM");
	auto transforms = transformBuffer->extendedData;

	vector<Triangle> triangles;
	vec3 dir(1, 0, 0);
	vec3 min(INFINITY);
	vec3 max(-INFINITY);

	for (int i = 0; i < transforms.size(); i++)
	{
		mat4 t = transforms[i];
		vector<vec3> transformedPts;
#pragma omp parallel for schedule(dynamic, 10)
		for (int j = 0; j < vertices.size(); j++)
		{
			transformedPts.push_back(vec3(t * vec4(vertices[j].position, 1)));

			for (int k = 0; k < 3; k++)
			{
				if (transformedPts[transformedPts.size() - 1][k] < min[k])
				{
					min[k] = transformedPts[transformedPts.size() - 1][k];
				}
				else if (transformedPts[transformedPts.size() - 1][k] > max[k])
				{
					max[k] = transformedPts[transformedPts.size() - 1][k];
				}
			}
		}

#pragma omp parallel for schedule(dynamic, 10)
		for (int j = 0; j < indices.size(); j += 3)
		{
			triangles.push_back(Triangle(transformedPts[indices[j]], transformedPts[indices[j + 1]], transformedPts[indices[j + 2]]));
		}
	}

	vec3 boundSize = max - min;
	vec3 center = (max + min) / 2.0f;

#pragma omp parallel for schedule(dynamic, 500)
	for (int i = 0; i < sampleSize;)
	{
		vec3 point(((float)rand()) / RAND_MAX - 0.5f, ((float)rand()) / RAND_MAX - 0.5f, ((float)rand()) / RAND_MAX - 0.5f);
		point *= boundSize;
		point += center;

		int sum = 0;
		for (int j = 0; j < triangles.size(); j++)
		{
			if (triangles[j].intersection(point, dir) > 0.0f)
			{
				sum++;
			}
		}

		if (sum % 2 == 1)
		{
#pragma omp critical
			{
				samples.push_back(point);
				i++;
			}
		}
	}

	return samples;
}