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

	static float distanceToHalfEdge(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge);
	static float distanceToFacet(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::Facet & facet);

	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::HalfEdge & halfedge);
	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::Facet & facet);
	static bool containsHalfEdge( Geometry::HalfEdge & halfedge,  Geometry::Facet & facet);

	static void connectHalfEdges(std::vector<Geometry::HalfEdge*> & halfedges);

	static void printEdge(Geometry::HalfEdge* he);
	static void printFacet(Geometry::Facet* facet);

};

