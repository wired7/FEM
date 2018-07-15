#pragma once
#include "GeometricalMeshObjects.h"
#include "HalfSimplices.h"
#include "ReferencedGraphicsObject.h"

static class HalfSimplexRenderingUtils
{
public:
	static Graphics::DecoratedGraphicsObject* getRenderableVolumesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			  const vector<vec3>& positions,
																			  const vector<mat4>& transforms,
																			  ReferenceManager* refMan);

	static Graphics::DecoratedGraphicsObject* getRenderableFacetsFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			  const vector<vec3>& positions,
																			  const vector<mat4>& transforms,
																			  ReferenceManager* refMan);

	static Graphics::DecoratedGraphicsObject* getRenderableEdgesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			 const vector<vec3>& positions,
																			 const vector<mat4>& transforms,
																			 ReferenceManager* refMan);

	static mat4 getHalfEdgeTransform(Geometry::HalfSimplex<1, GLuint>* halfEdge,
									 const vector<vec3>& positions,
									 const mat4& parentTransform,
									 const vec3 centroid);
};

