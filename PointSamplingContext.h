#pragma once
#include "Context.h"
#include "PointSamplingController.h"

class PointSamplingController;

class PointSamplingContext : public GraphicsSceneContext<PointSamplingController, SphericalCamera, PointSamplingContext>
{
protected:
	bool pointsReady = false;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	vector<vec3> points;
	static vector<vec3> sampleSurface(int sampleSize, DecoratedGraphicsObject* object);
	PointSamplingContext(DecoratedGraphicsObject* surface, SphericalCamera* cam);
	~PointSamplingContext() {};
	virtual void update(void);
};