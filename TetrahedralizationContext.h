#pragma once
#include "Context.h"
#include "TetrahedralizationController.h"
#include "Halfsimplices.h"
#include "ReferencedGraphicsObject.h"
#include "ImplicitGeometry.h"

class TetrahedralizationController;

class TetrahedralizationContext : public GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>
{

protected:
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures) override;

private:
	vector<glm::vec3>& positions;
	vector<mat4> tetraTransforms;
public:
	bool tetrahedralizationReady = false;
	ReferenceManager* refMan;
	Geometry::Manifold3<GLuint> manifold;
	TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface,
							  Graphics::DecoratedGraphicsObject* points,
							  vector<vec3>& _points,
							  FPSCamera* cam,
							  ReferenceManager* refMan);
	~TetrahedralizationContext() {};
	void update(void) override;
	void updateGeometries();
};
