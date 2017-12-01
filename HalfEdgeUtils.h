#pragma once
#include "HalfSimplices.h"
#include "GraphicsObject.h"

static class HalfEdgeUtils
{
public:
	static vector<Geometry::HalfEdge*> getFacetHalfEdges(Geometry::Facet* facet);
	static vector<Geometry::Vertex*> getFacetVertices(Geometry::Facet* facet);
	static vec3 getFacetCentroid(Geometry::Facet* facet, Graphics::MeshObject* m, const mat4& parentTransform);
	static mat4 getHalfEdgeTransform(Geometry::HalfEdge* halfEdge, Graphics::MeshObject* m, const mat4& parentTransform, const vec3& centroid);
};

