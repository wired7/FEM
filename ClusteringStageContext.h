#pragma once
#include "Context.h"
#include "ClusteringStageController.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"

class ClusteringStageController;

class ClusteringStageContext : public GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>
{
protected:
	ReferenceManager* refMan;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
	virtual void update(void);
	virtual void computeRenderableBuffer(void);
public:
	ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, FPSCamera* cam);
	~ClusteringStageContext();
};

