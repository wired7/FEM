#pragma once
#include "ImplicitGeometry.h"
#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"
#include <utility>
#include <map>

using namespace std;
using namespace glm;

static class VoronoiDiagramUtils
{
public:
	static Sphere getCircumcircle(vec3 points[3]);
	static Sphere getCircumsphere(vec3 points[4]);
	static Geometry::Vertex* getVoronoiPointFromTetrahedron(Geometry::Mesh* mesh, const vector<vec3>& inputPositions, vector<vec3>& outputPositions);
	static pair<Geometry::Vertex*, Geometry::Vertex*> getVoronoiEdgeFromTetrahedraPair(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2, const map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	static Geometry::Facet* getVoronoiFacetFromEdge(pair<Geometry::Vertex*, Geometry::Vertex*> edgeVertices, const map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	static vector<pair<Geometry::Vertex*, Geometry::Vertex*>> getEdgesIncidentToVertex(Geometry::Vertex* vertex, Geometry::VolumetricMesh* volumetricMesh);

	static Geometry::VolumetricMesh* getVoronoiDiagram(Geometry::VolumetricMesh* volumetricMesh, const vector<vec3>& positions);
};

