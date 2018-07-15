#pragma once
#include "Context.h"
#include "PointSamplingController.h"
#include "ReferencedGraphicsObject.h"

class PointSamplingController;

class PointSamplingContext : public GraphicsSceneContext<PointSamplingController, FPSCamera, PointSamplingContext>
{
protected:
	bool pointsReady = false;
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
public:
	ReferenceManager * refMan;
	bool readyToAdvance = false;
	vector<vec3> points;
	static vector<vec3> sampleSurface(int sampleSize, Graphics::DecoratedGraphicsObject* object);
	PointSamplingContext(Graphics::DecoratedGraphicsObject* surface, FPSCamera* cam, ReferenceManager* refMan);
	~PointSamplingContext() {};
	void update(void) override;
};