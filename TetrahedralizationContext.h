#pragma once
#include "Context.h"
#include "TetrahedralizationController.h"

class TetrahedralizationController;
class HalfEdge;
class Facet;
class HalfFacet;
class Volume;

class TetrahedralizationContext : public GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>
{
protected:
	vector<mat4> tetrahedra;
	bool tetrahedralizationReady = false;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);

	vector<glm::vec3> & points;

//	vector<Vertex> vertices;
//	vector<HalfEdge> halfedges;
//	vector<Facet> facets;
//	vector<Volume> volumes;

public:
	TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, vector<vec3> & _points, SphericalCamera* cam);
	~TetrahedralizationContext() {};
	virtual void update(void);
};
