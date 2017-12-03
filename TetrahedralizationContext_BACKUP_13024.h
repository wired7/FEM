#pragma once
#include "Context.h"
#include "TetrahedralizationController.h"
#include "Halfsimplices.h"
#include "ReferencedGraphicsObject.h"

class TetrahedralizationController;


class TetrahedralizationContext : public GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>
{
<<<<<<< HEAD

=======
>>>>>>> 52ad980cbc372e20178c7062635f26616bfdc248
protected:
	vector<mat4> tetrahedra;
	bool tetrahedralizationReady = false;
	virtual void setupCameras(void) {};
	virtual void setupGeometries(void);
	virtual void setupPasses(void);

private:
	void initialTetrahedralization();
	Geometry::VolumetricMesh   volume;
	vector<glm::vec3>		 & positions;
	vector<Geometry::Facet*>   openFacets;
	vector<bool>			   usedVertices;
	ReferenceManager* refMan;

public:
	TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, Graphics::DecoratedGraphicsObject* points, vector<vec3> & _points, SphericalCamera* cam);
	~TetrahedralizationContext() {};
	virtual void update(void);
<<<<<<< HEAD
	bool addNextTetra();
	bool fillUpGaps();
	void updateGeometries();
=======
	bool addNextFacet();
	void updateGeometries();



>>>>>>> 52ad980cbc372e20178c7062635f26616bfdc248


};
