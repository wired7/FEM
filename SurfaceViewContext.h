#pragma once
#include "Context.h"
#include "SurfaceViewController.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"


class SurfaceViewController;

class SurfaceViewContext : public GraphicsSceneContext<SurfaceViewController, FPSCamera, SurfaceViewContext>
{
private:
	void setupRenderableHalfEdges(Geometry::Manifold2<GLuint>* manifold, const vector<vec3>& positions, const vector<mat4>& transform);
	void setupRenderableVertices(const vector<vec3>& positions, const vector<mat4>& transform);
	void setupRenderableFacets(Geometry::Manifold2<GLuint>* manifold, const vector<vec3>& positions, const vector<mat4>& transform);
protected:
	void setupCameras(void) override;
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
	void update(void) override;
public:
	ReferenceManager * refMan;
	SurfaceViewContext();
	~SurfaceViewContext() {};
};