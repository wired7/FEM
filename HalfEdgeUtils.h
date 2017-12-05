#pragma once
#include "HalfSimplices.h"
#include "GraphicsObject.h"
#include "ReferencedGraphicsObject.h"

class Triangle;
static class HalfEdgeUtils
{
public:
	static vector<Geometry::HalfEdge*> getFacetHalfEdges(Geometry::Facet* facet);
	static vector<Geometry::Vertex*> getFacetVertices(Geometry::Facet* facet);
	static vector<Geometry::Facet*> GetOpenFacets(Geometry::Mesh* mesh);
	static vector<Geometry::Mesh*> getNeighbouringMeshes(Geometry::Mesh* mesh);
	static vector<Geometry::Facet*> getNeighbouringFacets(Geometry::Facet* facet);

	static vector<Geometry::Mesh*> getVertexMeshes(Geometry::Vertex* vertex);
	static vector<Geometry::Mesh*> getEdgeMeshes(pair<Geometry::Vertex*, Geometry::Vertex*> edge);
	static vector<vector<Geometry::Mesh*>> HalfEdgeUtils::BreadthFirstSearch(Geometry::Mesh* mesh, int depth);
	static vector<vector<Geometry::Facet*>> HalfEdgeUtils::BreadthFirstSearch(Geometry::Facet* facet, int depth);

	static Triangle* facetToTriangle(Geometry::Facet* facet, vector<vec3> & positions);
	static bool vertexSeesFacet(Geometry::Vertex* vertex, Geometry::Facet* facet, vector<Triangle*> & triangles, vector<vec3>& positions);
	static int fireRay(vec3 source, vec3 destination, vector<Triangle*> & triangles, int exitNumber = -1);
	static vector<vec3> getVolumeVertices(Geometry::Mesh* mesh, const vector<vec3>& positions);

	static vec3 getFacetDirection(Geometry::Facet* facet, const vector<vec3> & positions);
	static vec3 getCorrectedFacetDirection(Geometry::Facet* facet, const vector<vec3> & positions);
	static bool facetsFaceEachother(Geometry::Facet* facet1, Geometry::Facet* facet2, vector<vec3> &positions);
	static vec3 getFacetCentroid(Geometry::Facet* facet, const vector<vec3>& positions, const mat4& parentTransform = {mat4(1.0f)});
	static vec3 getMeshCentroid(Geometry::Mesh* mesh, const vector<vec3>& positions);
	static vector<vec3> getFacetVertexPositions(Geometry::Facet* facet, vector<vec3> & positions);

	static mat4 getHalfEdgeTransform(Geometry::HalfEdge* halfEdge, const vector<vec3>& positions, const mat4& parentTransform, const vec3& centroid);
	static Graphics::DecoratedGraphicsObject* getRenderableFacetsFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, const vector<mat4>& transforms = {mat4(1.0f)});
	static Graphics::DecoratedGraphicsObject* getRenderableEdgesFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms = { mat4(1.0f) });
	static Graphics::DecoratedGraphicsObject* getRenderableVolumesFromMesh(Geometry::VolumetricMesh* mesh, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms = { mat4(1.0f) });

	static float distanceToHalfEdge(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge);
	static float distanceToFacet(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::Facet & facet);


	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::HalfEdge & halfedge);
	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::Facet & facet);
	static bool containsVertex( Geometry::Vertex & vertex,  Geometry::Mesh & mesh);
	static bool containsHalfEdge(Geometry::HalfEdge & halfedge, Geometry::Facet & facet);
	static bool findHalfEdgeTwin(Geometry::Facet* facet, Geometry::HalfEdge* halfedge);
	static bool areTwins(Geometry::Facet* facet1, Geometry::Facet* facet2);
	static pair<Geometry::Facet*,Geometry::Facet*> findFacetWithTwin(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2);


	static bool facetPointsTo(Geometry::Facet & facet, Geometry::Vertex & vertex, vector<vec3> & positions);

	static bool getOrientation(Geometry::Mesh* mesh, const vector<vec3>& positions);

	static vector<int> makeFacetPartition(Geometry::Facet * facet, vector<vec3> & positions, vector<int> currentPartition);
	static vector<int> getPartitionIntersection(vector<int> & partition1, vector<int> & partition2);
	static vector<int> getPartitionUnion(vector<int> & partition1, vector<int> & partition2);

	static vector<Geometry::Vertex*> getVertexIntersection(vector<Geometry::Vertex*> & vertices1, vector<Geometry::Vertex*> & vertices2);
	static vector<Geometry::Vertex*> getVertexIntersection(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2);
	static vector<Geometry::Vertex*> getVertexIntersection(Geometry::Facet* facet1, Geometry::Facet* facet2);

	static vector<Geometry::Vertex*> getVertexDifference(vector<Geometry::Vertex*> &vertices1, vector<Geometry::Vertex*> & vertices2);

	static float getFacetPointVolume(Geometry::Facet* facet, Geometry::Vertex* vertex, vector<vec3>& positions);
	static float getTetraVolume(Geometry::Mesh* mesh, vector<vec3> & positions);

	static vector<Geometry::Vertex*> connectHalfEdges(std::vector<Geometry::HalfEdge*> & halfedges);
	static vector<Geometry::HalfEdge*> connectFacets(std::vector<Geometry::Facet*> & facets, int NumVertices);
	
	static void printEdge(Geometry::HalfEdge* he);
	static void printFacet(Geometry::Facet* facet);
	static void printMesh(Geometry::Mesh * m);

	static Geometry::Facet* constructFacet(vector<Geometry::Vertex*> & vertices);
	static Geometry::Facet* constructTwinFacet(Geometry::Facet* facet);


	static Geometry::Mesh* constructTetrahedron(Geometry::Vertex & vertex, Geometry::Facet & facet,  vector<Geometry::Vertex*> & vertices, const vector<vec3>& positions);
	static Geometry::Mesh* constructTetrahedron( Geometry::Vertex* v1, Geometry::Vertex* v2, Geometry::Vertex* v3, Geometry::Vertex *v4, vector<Geometry::Vertex*> & allVertices, const vector<vec3>& positions);
	static Geometry::Mesh* constructTetrahedron(Geometry::Vertex & vertex, Geometry::Facet  & facet1, Geometry::Facet & facet2, vector<Geometry::Vertex*> & vertices, const vector<vec3>& positions);

	static void addMeshToVolume(Geometry::Mesh* mesh, Geometry::VolumetricMesh *volume);
};

