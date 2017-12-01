#include "HalfEdgeUtils.h"
#include <iostream>
#include "LinearAlgebraUtils.h"

using namespace Geometry;
vector<Geometry::HalfEdge*> HalfEdgeUtils::getFacetHalfEdges(Geometry::Facet* facet)
{
	vector<Geometry::HalfEdge*> edges;

	Geometry::HalfEdge* halfEdge = facet->halfEdge;
	Geometry::HalfEdge* newEdge = halfEdge;
	while (true) {
		edges.push_back(newEdge);
		newEdge = newEdge->next;
		if (newEdge == halfEdge)
		{
			break;
		}
	}
	return edges;
}

vector<Geometry::Vertex*> HalfEdgeUtils::getFacetVertices(Geometry::Facet* facet)
{
	auto edges = getFacetHalfEdges(facet);
	vector<Geometry::Vertex*> vertices;
	for (int i = 0; i < edges.size(); i++)
	{
		vertices.push_back(edges[i]->vertex);
	}

	return vertices;
}
static float distanceToHalfEdge(const vector<vec3> & positions, const Geometry::Vertex & vertex, Geometry::HalfEdge halfedge) {

	const vec3 & vPos = positions[vertex.externalIndex];
	const vec3 & hPos1 = positions[halfedge.start];
	const vec3 & hPos2 = positions[halfedge.end];

	float f1 = glm::distance(hPos1, vPos);
	float f2 = glm::distance(hPos2, vPos);

	return sqrtf(f1 + f2);
}

vec3 HalfEdgeUtils::getFacetCentroid(Geometry::Facet* facet, Graphics::MeshObject* m, const mat4& parentTransform)
{
	vec3 centroid(0.0f);
	
	auto edges = getFacetHalfEdges(facet);

	for (int i = 0; i < edges.size(); i++)
	{
		centroid += vec3(parentTransform * vec4(m->vertices[edges[i]->vertex->externalIndex].position, 1));
	}

	return centroid / (float)edges.size();
}

mat4 HalfEdgeUtils::getHalfEdgeTransform(Geometry::HalfEdge* halfEdge, Graphics::MeshObject* m, const mat4& parentTransform, const vec3& centroid)
{
	vec3 point[2];
	point[0] = vec3(parentTransform * vec4(m->vertices[halfEdge->start].position, 1));
	point[1] = vec3(parentTransform * vec4(m->vertices[halfEdge->end].position, 1));
	float edgeLength = length(point[1] - point[0]);
	auto lineTransform = LinearAlgebraUtils::getLineTransformFrom2Points(point[0], point[1]);

	mat4 aroundCentroid =
		translate(mat4(1.0f), -centroid) *
		lineTransform *
		scale(mat4(1.0f), vec3(edgeLength, edgeLength * 0.01f, edgeLength * 0.01f)) *
		translate(mat4(1.0f), vec3(0.5f, 0, 0)) *
		scale(mat4(1.0f), vec3(0.9f)) *
		translate(mat4(1.0f), vec3(-0.5f, 0, 0));

	mat4 transform =
		translate(mat4(1.0f), centroid) *
		scale(mat4(1.0f), vec3(0.8f)) *
		aroundCentroid;

	return transform;
}