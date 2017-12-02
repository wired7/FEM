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
	void setupRenderableHalfEdges(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions);
	void setupRenderableVertices(const vector<vec3>& positions);
	void setupRenderableFacets(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions);
protected:
	virtual void setupCameras(void);
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
public:
	SurfaceViewContext();
	~SurfaceViewContext() {};
};