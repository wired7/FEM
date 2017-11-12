#pragma once
#include "Context.h"
#include "TetrahedralizationController.h"

class TetrahedralizationController;

class TetrahedralizationContext : public GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>
{
protected:
	vector<mat4> tetrahedra;
	bool tetrahedralizationReady = false;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	TetrahedralizationContext(DecoratedGraphicsObject* surface, vector<vec3> points, SphericalCamera* cam);
	~TetrahedralizationContext() {};
	virtual void update(void);
};
