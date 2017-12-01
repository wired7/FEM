#include "HalfEdgeUtils.h"
#include <iostream>

vector<HalfEdge::HalfEdge*> HalfEdgeUtils::getFacetHalfEdges(HalfEdge::Facet* facet)
{
	vector<HalfEdge::HalfEdge*> edges;

	HalfEdge::HalfEdge* halfEdge = facet->halfEdge;
	HalfEdge::HalfEdge* newEdge = halfEdge;
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

vec3 HalfEdgeUtils::getFacetCentroid(HalfEdge::Facet* facet, MeshObject* m, const mat4& parentTransform)
{
	vec3 centroid(0.0f);
	
	auto edges = getFacetHalfEdges(facet);

	for (int i = 0; i < edges.size(); i++)
	{
		centroid += vec3(parentTransform * vec4(m->vertices[edges[i]->vertex->externalIndex].position, 1));
	}

	return centroid / (float)edges.size();
}

mat4 HalfEdgeUtils::getHalfEdgeTransform(HalfEdge::HalfEdge* halfEdge, MeshObject* m, const mat4& parentTransform, const vec3& centroid)
{
	vec3 point[2];
	point[0] = vec3(parentTransform * vec4(m->vertices[halfEdge->start].position, 1));
	point[1] = vec3(parentTransform * vec4(m->vertices[halfEdge->end].position, 1));
	float edgeLength = length(point[1] - point[0]);
	vec3 vectorDir = normalize(point[1] - point[0]);
	vec3 z;
	if (abs(vectorDir.x) == 0.0f || (vectorDir.y != 0.0f && vectorDir.z != 0.0f))
	{
		z = -normalize(cross(vectorDir, vec3(1, 0, 0)));
	}
	else if (vectorDir.x == 1.0f)
	{
		z = vec3(0, 0, 1);
	}
	else
	{
		z = vec3(0, 0, -1);
	}

	float angle = acos(dot(normalize(point[1] - point[0]), vec3(1, 0, 0)));

	mat4 aroundCentroid =
		translate(mat4(1.0f), point[0] - centroid) *
		rotate(mat4(1.0f), angle, z) *
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