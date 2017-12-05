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
	CLBuffer<int>* offsetBuffer;
	CLBuffer<int>* sizeBuffer;
	CLBuffer<float>* cameraPosBuffer;
	CLBuffer<float>* cameraDirBuffer;
	CLGLBuffer<float>* renderableBuffer;
	CLGLBuffer<float>* positionsBuffer;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
	virtual void update(void);
	virtual void computeRenderableBuffer(DecoratedGraphicsObject* volume);
public:
	CLBuffer<float>* separationDistanceBuffer;
	bool updateDistanceBuffer;
	ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, Geometry::VolumetricMesh* mesh, FPSCamera* cam);
	~ClusteringStageContext();
};