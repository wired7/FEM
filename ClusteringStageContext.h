#pragma once
#include "Context.h"
#include "ClusteringStageController.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"
#include "OpenCLContext.h"

class ClusteringStageController;

class ClusteringStageContext : public GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>
{
protected:
	ReferenceManager* refMan;
	OpenCLContext* clContext;
	CLKernel* volumeUpdateKernel;
	CLBuffer<float>* cameraPosBuffer;
	CLBuffer<float>* cameraDirBuffer;
	CLGLBuffer<float>* renderableBuffer;
	CLGLBuffer<float>* positionsBuffer;
	CLGLBuffer<mat4>* transformsBuffer;
	vec4 color1 = vec4(0, 0, 1, 1);
	vec4 color2 = vec4(1, 1, 0, 1);
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
	void update(void) override;

public:
	CLBuffer<float>* separationDistanceBuffer;
	bool updateDistanceBuffer;
	ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, Geometry::Manifold3<GLuint>* manifold, FPSCamera* cam);
	~ClusteringStageContext();
};