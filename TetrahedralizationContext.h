#pragma once
#include "Context.h"
#include "TetrahedralizationController.h"
#include "Halfsimplices.h"
#include "ReferencedGraphicsObject.h"

class TetrahedralizationController;


class TetrahedralizationContext : public GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>
{

protected:
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);

private:
	void initialTetrahedralization();
	vector<glm::vec3>		 & positions;
	vector<Geometry::Facet*>   openFacets;
	vector<bool>			   usedVertices;
	ReferenceManager* refMan;
	vector<vector<int>> partitions;
	int currentInd;
	vector<ivec4> tetrahedra;
public:
	bool tetrahedralizationReady = false;
	TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, Graphics::DecoratedGraphicsObject* points, vector<vec3> & _points, FPSCamera* cam);
	~TetrahedralizationContext() {};
	virtual void update(void);
	bool addNextTetra();
	bool addTetraFromGraph();
	vector<Geometry::Mesh*> fillUpGaps(Geometry::Mesh* mesh);
	void updateGeometries();

	Geometry::VolumetricMesh   volume;


};
