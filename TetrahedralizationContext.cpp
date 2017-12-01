#pragma once

#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include <thread>
#include "HalfSimplices.h"

using namespace Geometry;

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, vector<vec3> &_points, SphericalCamera* cam) : positions(_points)
{
	cameras.push_back(cam);
	geometries.push_back(surface);

	setupGeometries();
	initialTriangulation();

	setupPasses();
	
	thread t([=] {
		// compute tetrahedralization here
		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{
	volume.meshes.push_back(new Mesh());
	vector<Vertex*>& vertices = volume.meshes[0]->vertices;

	vertices.resize(positions.size());
	for (int i = 0; i < vertices.size();i++) {
		vertices[i] = new Vertex(i);
	}
}

void TetrahedralizationContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("C") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void TetrahedralizationContext::initialTriangulation(void) {
	
	Mesh * mesh = volume.meshes[0];
	vector<Vertex*> &vertices = mesh->vertices;
	Vertex * seed = vertices[0];
	Vertex * nearest = vertices[1];

	vec3 posSeed  = positions[seed->externalIndex];
	vec3 posNearest = positions[nearest->externalIndex];

	float shortestDistance = glm::distance(posSeed, posNearest);
	// first first nearest neighbour;

	for (int i = 1; i < vertices.size();i++) {
		Vertex* nextVert = vertices[i];
		vec3 pos = positions[nextVert->externalIndex];
		float distance = glm::distance(posSeed, pos);

		if (distance < shortestDistance) {
			shortestDistance = distance;
			posNearest = pos;
			nearest = nextVert;
		}
	
	}

	printf("Closest point to (%f, %f, %f) is (%f, %f, %f)\n\n", 
		posSeed.x, posSeed.y, posSeed.z, posNearest.x, posNearest.y, posNearest.z);

}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		setupGeometries();
		tetrahedralizationReady = false;
	}
}