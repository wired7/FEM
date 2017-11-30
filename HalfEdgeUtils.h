#pragma once
#include "HalfSimplices.h"
#include "GraphicsObject.h"

static class HalfEdgeUtils
{
public:
	static vector<HalfEdge::HalfEdge*> getFacetHalfEdges(HalfEdge::Facet* facet);
	static vec3 getFacetCentroid(HalfEdge::Facet* facet, MeshObject* m, const mat4& parentTransform);
	static mat4 getHalfEdgeTransform(HalfEdge::HalfEdge* halfEdge, MeshObject* m, const mat4& parentTransform, const vec3& centroid);
};

