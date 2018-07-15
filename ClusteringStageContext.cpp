#pragma once
#include "ClusteringStageContext.h"
#include "ClusteringStageController.h"

#include "HalfSimplices.h"
#include "FPSCameraControls.h"
#include "WindowContext.h"


ClusteringStageContext::ClusteringStageContext(Graphics::DecoratedGraphicsObject* mesh, Geometry::Manifold3<GLuint>* manifold, FPSCamera* cam)
{
	cameras.push_back(cam);
	geometries["VOLUMES"] = mesh;
	setupGeometries();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	clContext = new OpenCLContext(WindowContext::window);
	volumeUpdateKernel = new CLKernel("volume_update_kernel.cl", "update_volume", clContext);

	vector<float> cameraPos = { 0, 0, 0 };
	vector<float> cameraDir = { 1, 0, 0 };
	vector<float> distance = { 3.0f };
	cameraPosBuffer = new CLBuffer<float>(clContext, cameraPos, CL_MEM_READ_WRITE);
	cameraDirBuffer = new CLBuffer<float>(clContext, cameraDir, CL_MEM_READ_WRITE);
	separationDistanceBuffer = new CLBuffer<float>(clContext, distance, CL_MEM_READ_WRITE);
	transformsBuffer = new CLGLBuffer<mat4>(
		clContext,
		geometries["VOLUMES"]->signatureLookup("TETRAINSTANCES")->VBO,
		CL_MEM_READ_WRITE);
	renderableBuffer = new CLGLBuffer<float>(clContext, geometries["VOLUMES"]->VBO, CL_MEM_READ_WRITE);
	renderableBuffer->bufferData.resize(((InstancedMeshObject<float, float>*)geometries["VOLUMES"]->signatureLookup("RENDERABLE"))->extendedData.size());

	auto vertices = (MeshObject*)geometries["VOLUMES"]->signatureLookup("VERTEX");
	positionsBuffer = new CLGLBuffer<float>(clContext, vertices->VBO);
	positionsBuffer->bufferData.resize(vertices->vertices.size());

	cameraPosBuffer->bindBuffer();
	cameraDirBuffer->bindBuffer();
	separationDistanceBuffer->bindBuffer();

	setupPasses({ "V1", "V2" }, { "SB" });
}


ClusteringStageContext::~ClusteringStageContext()
{
}

void ClusteringStageContext::setupGeometries(void)
{
	refMan = new ReferenceManager();
	makeQuad();

	auto instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)geometries["VOLUMES"]->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	vector<float> renderable;
	for (int i = 0; i < objectIDs.size(); i++)
	{
		renderable.push_back(1.0f);
	}

	geometries["VOLUMES"] = new InstancedMeshObject<float, float>(geometries["VOLUMES"], renderable, "RENDERABLE", 1);
	dirty = true;
}

void ClusteringStageContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> clKMap;
	clKMap["VOLUMEUPDATE"] = std::make_pair(volumeUpdateKernel, std::make_pair(renderableBuffer->bufferData.size(), 2));
	CLPass* clP = new CLPass(clKMap);

	clP->addBuffer(renderableBuffer, "RENDERABLE", "VOLUMEUPDATE", 0, true);
	clP->addBuffer(positionsBuffer, "POSITIONS", "VOLUMEUPDATE", 1, true);
	clP->addBuffer(cameraPosBuffer, "CAMERAPOS", "VOLUMEUPDATE", 2);
	clP->addBuffer(cameraDirBuffer, "CAMERADIR", "VOLUMEUPDATE", 3);
	clP->addBuffer(separationDistanceBuffer, "SEPARATIONDISTANCE", "VOLUMEUPDATE", 4);
	clP->addBuffer(transformsBuffer, "TRANSFORMS", "VOLUMEUPDATE", 5, true);

	map<string, ShaderProgramPipeline*> gPrograms1;
	gPrograms1["V1"] = ShaderProgramPipeline::getPipeline("V1");

	GeometryPass* gP1 = new GeometryPass(gPrograms1, "GEOMETRYPASS0", nullptr, 1, 1, false);
	gP1->setupCamera(cameras[0]);

	clP->addNeighbor(gP1);

	map<string, ShaderProgramPipeline*> gPrograms2;
	gPrograms2["V2"] = ShaderProgramPipeline::getPipeline("V2");

	GeometryPass* gP2 = new GeometryPass(gPrograms2, "GEOMETRYPASS1");
	gP2->setupCamera(cameras[0]);

	clP->addNeighbor(gP2);

	map<string, ShaderProgramPipeline*> lPrograms;

	for (const auto& programSignature : lProgramSignatures)
	{
		lPrograms[programSignature] = ShaderProgramPipeline::getPipeline(programSignature);
	}

	LightPass* lP = new LightPass(lPrograms, true);

	gP1->addNeighbor(lP);
	gP2->addNeighbor(lP);

	passRootNode = clP;

	gP1->addRenderableObjects(geometries["VOLUMES"], "V1");
	gP2->addRenderableObjects(geometries["VOLUMES"], "V2");
	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "SB");

	gP1->setupVec4f(color1, "inputColor");
	gP2->setupVec4f(color2, "inputColor");
}

void ClusteringStageContext::update(void)
{
	//	if (length(cameras[0]->velocity) > 0)
	{
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

	GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>::update();

	dirty = true;
}