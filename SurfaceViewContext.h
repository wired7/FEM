#pragma once
#include "Context.h"
#include "SurfaceViewController.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"


class SurfaceViewController;

class SurfaceViewContext : public GraphicsSceneContext<SurfaceViewController, FPSCamera, SurfaceViewContext>
{
private:
	ReferenceManager* refMan;
	void setupRenderableHalfEdges(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions, const vector<mat4>& transform);
	void setupRenderableVertices(const vector<vec3>& positions, const vector<mat4>& transform);
	void setupRenderableFacets(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions, const vector<mat4>& transform);
protected:
	virtual void setupCameras(void);
	virtual void setupGeometries(void);
	virtual void setupPasses(void);
	virtual void update(void);
public:
	SurfaceViewContext();
	~SurfaceViewContext() {};
};