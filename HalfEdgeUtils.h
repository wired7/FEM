#pragma once
#include "HalfSimplices.h"
#include "GraphicsObject.h"
#include "ReferencedGraphicsObject.h"

static class HalfEdgeUtils
{
public:
	static vector<Geometry::HalfEdge*> getFacetHalfEdges(Geometry::Facet* facet);
	static vector<Geometry::Vertex*> getFacetVertices(Geometry::Facet* facet);
	static vector<vec3> getVolumeVertices(Geometry::Mesh* mesh, const vector<vec3>& positions);
	static float distanceToHalfEdge(const vector<vec3> & positions, const Geometry::Vertex & vertex, Geometry::HalfEdge halfedge);
	static vec3 getFacetCentroid(Geometry::Facet* facet, const vector<vec3>& positions, const mat4& parentTransform);
	static mat4 getHalfEdgeTransform(Geometry::HalfEdge* halfEdge, const vector<vec3>& positions, const mat4& parentTransform, const vec3& centroid);
	static Graphics::DecoratedGraphicsObject* getRenderableFacetsFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, const vector<mat4>& transforms = {mat4(1.0f)});
	static Graphics::DecoratedGraphicsObject* getRenderableEdgesFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms = { mat4(1.0f) });
	static Graphics::DecoratedGraphicsObject* getRenderableVolumesFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms = { mat4(1.0f) });

	static float distanceToHalfEdge(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge);
	static float distanceToFacet(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::Facet & facet);

	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::HalfEdge & halfedge);
	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::Facet & facet);
	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::Mesh & mesh);

	static bool containsHalfEdge( Geometry::HalfEdge & halfedge,  Geometry::Facet & facet);

	static vector<Geometry::Vertex*> connectHalfEdges(std::vector<Geometry::HalfEdge*> & halfedges);
	static vector<Geometry::HalfEdge*> connectFacets(std::vector<Geometry::Facet*> & facets, int NumVertices);
	
	static void printEdge(Geometry::HalfEdge* he);
	static void printFacet(Geometry::Facet* facet);
	static void printMesh(Geometry::Mesh * m);

	static Geometry::Facet* constructFacet(vector<Geometry::Vertex*> & vertices);
	static Geometry::Facet* constructTwinFacet(Geometry::Facet* facet);

	static Geometry::Mesh* constructTetrahedron(Geometry::Vertex & vertex, Geometry::Facet & facet,  vector<Geometry::Vertex*> vertices);

};

