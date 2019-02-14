#pragma once
#include "SurfaceViewContext.h"
#include "HalfSimplexRenderingUtils.h"
#include "FPSCameraControls.h"

using namespace Graphics;
SurfaceViewContext::SurfaceViewContext() : GraphicsSceneContext()
{
	setupCameras();
	setupGeometries();
	setupPasses({ "A", "EdgeA", "C", "D" }, {"B"});
}

void SurfaceViewContext::setupCameras(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	cameras.push_back(new FPSCamera(WindowContext::window, vec2(0, 0), vec2(1, 1), vec3(0, 0, 5), vec3(0, 0, 0),
		vec3(0, 1, 0), perspective(45.0f, (float)width / height, 0.1f, 1000.0f)));
}

void SurfaceViewContext::setupGeometries(void)
{
	refMan = new ReferenceManager();
	makeQuad();

	auto meshObject = new ImportedMeshObject("models\\filledChinchilla.obj");

/*	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, meshObject, meshObject->indices.size() / 3, "INSTANCEID", 3);

	vector<GLbyte> selectedC;

	for (int i = 0; i < meshObject->indices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::ExtendedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION");

	vector<mat4> parentTransforms;
	parentTransforms.push_back(scale(mat4(1.0f), vec3(10.0f)));

	geometries["VOLUME"] = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM");*/
//	auto m = new Cylinder(10);//Polyhedron(10, vec3(), vec3(1.0f));
//	auto meshObject = new Tetrahedron();

	vector<mat4> transform;

	vec3 pos = vec3(0, 0, 0);
	transform.push_back(translate(mat4(1.0f), pos) * scale(mat4(1.0f), vec3(10.0f)));

	Geometry::Manifold2<GLuint>* manifold = new Geometry::Manifold2<GLuint>(meshObject->indices);

	vector<vec3> positions;
	for (int i = 0; i < meshObject->vertices.size(); i++)
	{
		positions.push_back(meshObject->vertices[i].position);
	}

	setupRenderableVertices(positions, transform);

	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableVolumesFromManifold(manifold,
																												   positions,
																												   transform,
																												   refMan);

	geometries["VOLUME"] = outputGeometry;

	setupRenderableHalfEdges(manifold, positions, transform);
	setupRenderableFacets(manifold, positions, transform);
}

void SurfaceViewContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	auto gP = ((GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS"));
	
	gP->addRenderableObjects(geometries["VOLUME"], "A");
	gP->addRenderableObjects(geometries["VERTICES"], "C");
	gP->addRenderableObjects(geometries["EDGES"], "EdgeA");
	gP->addRenderableObjects(geometries["FACETS"], "D");
	
	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "B");
}

void SurfaceViewContext::setupRenderableHalfEdges(Geometry::Manifold2<GLuint>* manifold, const vector<vec3>& positions, const vector<mat4>& transform)
{
	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableEdgesFromManifold(manifold,
																												  positions,
																												  transform,
																												  refMan);

	geometries["EDGES"] = outputGeometry;
}

void SurfaceViewContext::setupRenderableVertices(const vector<vec3>& positions, const vector<mat4>& transform)
{
	vector<mat4> pTransforms;

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < positions.size(); i++)
		{
			pTransforms.push_back(transform[j] * scale(mat4(1.0f), vec3(1.0f)) * translate(mat4(1.0f), positions[i]) * scale(mat4(1.0f), vec3(0.002f)));
		}
	}

	auto point = new Polyhedron(4, vec3(), vec3(1.0f));

	auto g = new MatrixInstancedMeshObject<mat4, float>(point, pTransforms, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, pTransforms.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < pTransforms.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries["VERTICES"] = selectable;
}

void SurfaceViewContext::setupRenderableFacets(Geometry::Manifold2<GLuint>* manifold, const vector<vec3>& positions, const vector<mat4>& transform)
{
	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableFacetsFromManifold(manifold,
																												   positions,
																												   transform,
																												   refMan);

	geometries["FACETS"] = outputGeometry;
}

void SurfaceViewContext::update(void)
{
	GraphicsSceneContext<SurfaceViewController, FPSCamera, SurfaceViewContext>::update();

	if (length(cameras[0]->velocity) > 0) {
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}
}