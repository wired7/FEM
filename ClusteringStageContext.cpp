#pragma once
#include "ClusteringStageContext.h"
#include "ClusteringStageController.h"

#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"

#include "FPSCameraControls.h"


ClusteringStageContext::ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, FPSCamera* cam)
{
	cameras.push_back(cam);

	setupGeometries();

	geometries.push_back(volume);

	setupPasses();
}


ClusteringStageContext::~ClusteringStageContext()
{
}

void ClusteringStageContext::setupGeometries(void)
{
	refMan = new ReferenceManager();
}

void ClusteringStageContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("V") });
	gP->addRenderableObjects(geometries[0], 0);

	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void ClusteringStageContext::update(void)
{
	GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>::update();

	if (length(cameras[0]->velocity) > 0) {
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}
}

void ClusteringStageContext::computeRenderableBuffer(void)
{

}