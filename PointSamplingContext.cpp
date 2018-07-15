#pragma once
#include "PointSamplingContext.h"
#include "DiscreteGeometryUtils.h"
#include "FPSCameraControls.h"
#include <thread>
#include <random>

using namespace std;
using namespace Graphics;

#define NUM_POINTS 100
#define POINT_SCALE 0.05f

PointSamplingContext::PointSamplingContext(DecoratedGraphicsObject* surface, FPSCamera* cam, ReferenceManager* refMan) : refMan(refMan)
{
	cameras.push_back(cam);
	geometries["SURFACE"] = surface;
	makeQuad();
	setupPasses({ "A", "C" }, {"B"});

	thread t([&]
	{
		points = sampleSurface(NUM_POINTS, surface);
		pointsReady = true;
	});
	t.detach();
}

void PointSamplingContext::setupGeometries(void)
{
	((GeometryPass*)passRootNode)->clearRenderableObjects("C");

	auto m = new Polyhedron(10, vec3(), vec3(1.0f));

	vector<mat4> transform;

	for (int i = 0; i < points.size(); i++)
	{
		transform.push_back(translate(mat4(1.0f), points[i]) * scale(mat4(1.0f), glm::vec3(1,1,1) * POINT_SCALE));
	}

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, transform, "OFFSET");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, points.size(), "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries["VERTICES"] = selectable;

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, "C");
	dirty = true;
}

void PointSamplingContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	((GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS"))->addRenderableObjects(geometries["SURFACE"], "A");
	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "B");
}

void PointSamplingContext::update(void)
{
	GraphicsSceneContext<PointSamplingController, FPSCamera, PointSamplingContext>::update();

	if (length(cameras[0]->velocity) > 0)
	{
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}

	if (pointsReady)
	{
		setupGeometries();
		pointsReady = false;
		readyToAdvance = true;
	}
}

vector<vec3> PointSamplingContext::sampleSurface(int sampleSize, DecoratedGraphicsObject* object)
{
	auto boundingBox = DiscreteGeometryUtils::getBoundingBox(object);
	vec3 boundSize = boundingBox.second - boundingBox.first;
	vec3 center = (boundingBox.second + boundingBox.first) / 2.0f;

	auto triangles = DiscreteGeometryUtils::getTrianglesFromMesh(object);
	vector<vec3> samples;
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<double> distribution(-0.5, 0.5);
	while (samples.size() < NUM_POINTS)
	{
		sampleSize = NUM_POINTS - samples.size();
#pragma omp parallel for schedule(dynamic, 10)
		for (int i = 0; i < sampleSize; ++i)
		{
			vec3 point(distribution(generator), distribution(generator), distribution(generator));
			point *= boundSize;
			point += center;

			if (DiscreteGeometryUtils::isPointInsideMesh(point, triangles))
			{
#pragma omp critical
				{
					samples.push_back(point);
				}
			}
		}
	}

	return samples;
}