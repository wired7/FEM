#pragma once
#include "ClusteringStageContext.h"
#include "ClusteringStageController.h"

#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"

#include "FPSCameraControls.h"
#include "WindowContext.h"


ClusteringStageContext::ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, Geometry::VolumetricMesh* mesh, FPSCamera* cam)
{
	dirty = true;
	cameras.push_back(cam);

	setupGeometries();

	clContext = new OpenCLContext(WindowContext::window);
	volumeUpdateKernel = new CLKernel("volume_update_kernel.cl", "update_volume", clContext);

	vector<int> offsets = { 0 };
	vector<int> sizes;
	vector<float> renderable;

	auto instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)volume->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	int currentIndex = objectIDs[0];
	int currentManagedIndex = refMan->assignNewGUID();
	int sizeCount = 0;

	for (int i = 0; i < objectIDs.size(); i++)
	{
		sizeCount++;

		if (objectIDs[i] != currentIndex)
		{
			currentIndex = objectIDs[i];
			currentManagedIndex = refMan->assignNewGUID();
			sizes.push_back(sizeCount);
			sizeCount = 0;
			offsets.push_back(i);
		}

		objectIDs[i] = currentManagedIndex;
		renderable.push_back(1.0f);
	}

	sizes.push_back(sizeCount);

	instanceIDs->extendedData = objectIDs;
	instanceIDs->updateBuffers();

	vector<float> cameraPos = { 0, 0, 0 };
	vector<float> cameraDir = { 1, 0, 0 };
	vector<float> distance = { 3.0f };
	offsetBuffer = new CLBuffer<int>(clContext, offsets, CL_MEM_READ_WRITE);
	sizeBuffer = new CLBuffer<int>(clContext, sizes, CL_MEM_READ_WRITE);
	cameraPosBuffer = new CLBuffer<float>(clContext, cameraPos, CL_MEM_READ_WRITE);
	cameraDirBuffer = new CLBuffer<float>(clContext, cameraDir, CL_MEM_READ_WRITE);
	separationDistanceBuffer = new CLBuffer<float>(clContext, distance, CL_MEM_READ_WRITE);

	offsetBuffer->bindBuffer();
	sizeBuffer->bindBuffer();
	cameraPosBuffer->bindBuffer();
	cameraDirBuffer->bindBuffer();
	separationDistanceBuffer->bindBuffer();

	clFinish(clContext->commandQueues[0]);

	auto renderableData = new ExtendedMeshObject<float, float>(volume, renderable, "RENDERABLE");
	auto vertices = ((MeshObject*)volume->signatureLookup("VERTEX"));

	glFinish();

	renderableBuffer = new CLGLBuffer<float>(clContext, renderableData->VBO, CL_MEM_READ_WRITE);
	renderableBuffer->bufferData.resize(renderable.size());

	positionsBuffer = new CLGLBuffer<float>(clContext, vertices->VBO);
	positionsBuffer->bufferData.resize(vertices->vertices.size());


	offsetBuffer->enableBuffer(volumeUpdateKernel, 0);
	sizeBuffer->enableBuffer(volumeUpdateKernel, 1);
	renderableBuffer->enableBuffer(volumeUpdateKernel, 2);
	positionsBuffer->enableBuffer(volumeUpdateKernel, 3);
	cameraPosBuffer->enableBuffer(volumeUpdateKernel, 4);
	cameraDirBuffer->enableBuffer(volumeUpdateKernel, 5);
	separationDistanceBuffer->enableBuffer(volumeUpdateKernel, 6);

	geometries.push_back(renderableData);

	setupPasses();

	auto bfsOutput = HalfEdgeUtils::BreadthFirstSearch(mesh->meshes[0], 10000);

	int returnedSize = bfsOutput.size();

	auto colorBuffer = ((ExtendedMeshObject<vec4, float>*)volume->signatureLookup("COLORS"));
	auto colors = colorBuffer->extendedData;

	int gapSize = 12;
	for (int i = 0; i < bfsOutput.size(); i++)
	{
		float color = (float)i / bfsOutput.size();
		for (int j = 0; j < bfsOutput[i].size(); j++)
		{
			int meshIndex = bfsOutput[i][j]->internalIndex * gapSize;
			for (int k = 0; k < gapSize; k++)
			{
				colors[meshIndex + k][0] += color;
			}
		}
	}

	colorBuffer->updateBuffers();
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

		for (int i = 0; i < 3; i++)
		{
			cameraPosBuffer->bufferData[i] = cameras[0]->camPosVector[i];
			cameraDirBuffer->bufferData[i] = cameras[0]->lookAtVector[i];
		}

		cameraPosBuffer->updateBuffer();
		cameraDirBuffer->updateBuffer();
	}

	if (updateDistanceBuffer)
	{
		separationDistanceBuffer->updateBuffer();
		updateDistanceBuffer = false;
	}

	glFinish();

	clEnqueueAcquireGLObjects(clContext->commandQueues[0], 1, &renderableBuffer->bufferPointer, 0, 0, 0);
	clEnqueueAcquireGLObjects(clContext->commandQueues[0], 1, &positionsBuffer->bufferPointer, 0, 0, 0);

	volumeUpdateKernel->execute(16 * (offsetBuffer->bufferData.size() / 8), 8);

	clFinish(clContext->commandQueues[0]);

	clEnqueueReleaseGLObjects(clContext->commandQueues[0], 1, &positionsBuffer->bufferPointer, 0, 0, 0);
	clEnqueueReleaseGLObjects(clContext->commandQueues[0], 1, &renderableBuffer->bufferPointer, 0, 0, 0);

	clFinish(clContext->commandQueues[0]);

	dirty = true;
}

void ClusteringStageContext::computeRenderableBuffer(DecoratedGraphicsObject* volume)
{

}