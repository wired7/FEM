#pragma once
#include "Context.h"
#include "PointSamplingController.h"

class PointSamplingController;

class PointSamplingContext : public GraphicsSceneContext<PointSamplingController, SphericalCamera, PointSamplingContext>
{
protected:
	vector<vec3> points;
	bool pointsReady = false;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	static vector<vec3> sampleSurface(int sampleSize, DecoratedGraphicsObject* object);
	PointSamplingContext(DecoratedGraphicsObject* surface, SphericalCamera* cam);
	~PointSamplingContext() {};
	virtual void update(void);
};