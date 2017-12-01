#pragma once
#include "Context.h"
#include "SurfaceViewController.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"


class SurfaceViewController;

class SurfaceViewContext : public GraphicsSceneContext<SurfaceViewController, SphericalCamera, SurfaceViewContext>
{
private:
	ReferenceManager* refMan;
	void setupRenderableHalfEdges(Geometry::HalfSimplices* hSimp, Graphics::DecoratedGraphicsObject* o);
	void setupRenderableVertices(Graphics::DecoratedGraphicsObject* o);
	void setupRenderableFacets(Geometry::HalfSimplices* hSimp, Graphics::DecoratedGraphicsObject* o);
protected:
	virtual void setupCameras(void);
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	SurfaceViewContext();
	~SurfaceViewContext() {};
};