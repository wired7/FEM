#pragma once
#include "Context.h"
#include "SurfaceViewController.h"
#include "ReferencedGraphicsObject.h"

class SurfaceViewController;

class SurfaceViewContext : public GraphicsSceneContext<SurfaceViewController, SphericalCamera, SurfaceViewContext>
{
private:
	ReferenceManager* refMan;
protected:
	virtual void setupCameras(void);
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	SurfaceViewContext();
	~SurfaceViewContext() {};
};