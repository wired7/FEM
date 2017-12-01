#pragma once
#include "TetrahedralizationContext.h"
#include <thread>

TetrahedralizationContext::TetrahedralizationContext(DecoratedGraphicsObject* surface, vector<vec3> &_points, SphericalCamera* cam) : points(_points)
{
	cameras.push_back(cam);
	geometries.push_back(surface);
	points = _points;

	setupPasses();
	
	thread t([=] {
		// compute tetrahedralization here
		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{
/*	((GeometryPass*)passRootNode)->clearRenderableObjects(0);

	auto m = new Tetrahedron();

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, tetrahedra, "OFFSET");

	vector<GLuint> instanceID;

	for (int i = 1; i <= tetrahedra.size(); i++)
	{
		instanceID.push_back(i);
	}

	auto pickable = new InstancedMeshObject<GLuint, GLuint>(g, instanceID, "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < tetrahedra.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, 0);*/
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

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		setupGeometries();
		tetrahedralizationReady = false;
	}
}