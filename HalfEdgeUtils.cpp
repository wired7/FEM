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

float HalfEdgeUtils::distanceToHalfEdge(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge) {

	const vec3 & vPos = positions[vertex.externalIndex];
	const vec3 & hPos1 = positions[halfedge.start];
	const vec3 & hPos2 = positions[halfedge.end];

	float f1 = glm::distance(hPos1, vPos);
	float f2 = glm::distance(hPos2, vPos);

	return sqrtf(f1 + f2);
}

float HalfEdgeUtils::distanceToFacet(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::Facet & facet) {

	const vec3 & vPos = positions[vertex.externalIndex];
	const vector<Vertex*> vertecies = getFacetVertices(&facet);
	float dist = 0;
	for (int i = 0; i < vertecies.size();i++) {
		const vec3 & facetPos = positions[vertecies[i]->externalIndex];
		dist += distance(vPos, facetPos);
	}

	return dist;
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

bool HalfEdgeUtils::containsVertex( Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge) {
	return (vertex.externalIndex == halfedge.start || vertex.externalIndex == halfedge.end);
}

bool HalfEdgeUtils::containsVertex( Geometry::Vertex & vertex, Geometry::Facet & facet) {

	vector<HalfEdge*> halfedges = getFacetHalfEdges(&facet);
	
	for (int i = 0; i < halfedges.size();i++) {
		if (containsVertex(vertex, *halfedges[i])) {
			return true;
		}
	}
	return false;
}

bool HalfEdgeUtils::containsVertex(Vertex & vertex, Mesh & mesh) {
	for (int i = 0; i < mesh.facets.size();i++) {
		if (containsVertex(vertex, *mesh.facets[i])) {
			return true;
		}
	}
	return false;
}
bool HalfEdgeUtils::containsHalfEdge( Geometry::HalfEdge & halfedge, Geometry::Facet & facet) {
	vector<HalfEdge*> halfedges = getFacetHalfEdges(&facet);
	for (int i = 0; i < halfedges.size();i++) {
		if (&halfedge == halfedges[i]){
			return true;
		}
	}
	return false;
}



// this connects halfedges in the orther they are in the vector... it will not ensure that they connected correctly
vector<Vertex*> HalfEdgeUtils::connectHalfEdges(std::vector<HalfEdge*> & halfedges) {
	vector<Vertex*> vertices;
	 for (int i = 0; i < halfedges.size() - 1;i++) {
		 halfedges[i]->next = halfedges[i + 1];
		 vertices.push_back(halfedges[i]->vertex);
	 }
	 halfedges[halfedges.size() - 1]->next = halfedges[0];
	 vertices.push_back(halfedges[halfedges.size() - 1]->vertex);

	 for (int i = halfedges.size()-1; i >0;i--) {
		 halfedges[i]->previous = halfedges[i - 1];
	 }
	 halfedges[0]->previous = halfedges[halfedges.size() - 1];
	 return vertices;
 }

vector<HalfEdge*> HalfEdgeUtils::connectFacets(std::vector<Facet*> & facets, int numVertices) {
	vector<HalfEdge*> halfedges;

	for (int i = 0; i < facets.size() - 1;i++) {
		facets[i]->next = facets[i + 1];
	}
	facets[facets.size() - 1]->next = facets[0];
	for (int i = facets.size() - 1; i >0;i--) {
		facets[i]->previous = facets[i - 1];
	}
	facets[0]->previous = facets[facets.size() - 1];


	vector<vector<HalfEdge*>> halfedgeMap(numVertices);

	for (int i = 0; i < facets.size();i++) {
		vector<HalfEdge*> halfedges = getFacetHalfEdges(facets[i]);
	
		for (int j = 0; j < halfedges.size();j++) {
			HalfEdge* he = halfedges[j];
			bool mapped = false;
			for (int k = 0; k < halfedgeMap[he->start].size(); k++) {
				if (halfedgeMap[he->start][k]->end == he->end) {

					mapped = true;
					break;
				}
			}
			if (!mapped) {
				halfedgeMap[he->start].push_back(he);
				halfedges.push_back(he);
			}
		}
	}

	for (int i = 0; i < halfedges.size(); i++) {
		HalfEdge* he = halfedges[i];
		if (he->twin == nullptr) {
			vector<HalfEdge*> &list = halfedgeMap[he->end];
			for (int j = 0; j < list.size(); j++) {
				HalfEdge * heTemp = list[j];

				if (heTemp->end == he->start) {
					heTemp->twin = he;
					he->twin = heTemp;
					break;
				}
			}
		}
	}
	return halfedges;
}


Facet* HalfEdgeUtils::constructFacet(vector<Vertex*> &vertices) {

	vector<HalfEdge*> halfedges;
	for (int i = 0; i < vertices.size()-1;i++) {
		halfedges.push_back(new HalfEdge(vertices[i], vertices[i+1]));
	}
	halfedges.push_back(new HalfEdge(vertices[vertices.size()-1], vertices[0] ));

	connectHalfEdges(halfedges);

	return new Facet(halfedges[0]);
}

Facet* HalfEdgeUtils::constructTwinFacet(Facet* facet) {
	
	vector<Vertex*> vertices = getFacetVertices(facet);
	vector<Vertex*> reverse(vertices.size());

	for (int i = reverse.size() - 1; i >= 0; i--) {
		reverse[reverse.size() - (i + 1)] = vertices[i];
	}

	Facet* twin = constructFacet(reverse);

	twin->twin = facet;
	facet->twin = twin;
	return twin;
}

Mesh* HalfEdgeUtils::constructTetrahedron(Vertex & vertex, Facet & facet, vector<Vertex*> vertices) {

	vector<Facet*> facets;
	facets.push_back(&facet);
	HalfEdge * start = facet.halfEdge;
	HalfEdge * next = start;
	
	do {
		//opposite orientation
		
		Vertex* a = next->vertex;
		Vertex* b = next->previous->vertex;
		Vertex* c = &vertex;

		vector<Vertex*> vertices = { a,b,c };
		facets.push_back(constructFacet(vertices));
		next = next->next;
	} while (next != start);
	

	Mesh* mesh = new Mesh();
	mesh->halfEdges = connectFacets(facets, vertices.size());
	mesh->facets = facets;
	for (int i = 0; i < vertices.size();i++) {
		if (containsVertex(*vertices[i],*mesh)) {
			mesh->vertices.push_back(vertices[i]);
		}
	}

	return mesh;
}

void HalfEdgeUtils::printEdge(HalfEdge* he) {
	std::cout << "HE( " << he->start << ", " << he->end << ") ";
}

void HalfEdgeUtils::printFacet(Facet * facet) {

	vector<HalfEdge*> halfedges = getFacetHalfEdges(facet);
	std::cout << "FACET { ";
	for (int i = 0; i < halfedges.size();i++) {
		printEdge(halfedges[i]);
		std::cout << ", ";
	}
	std::cout << " }";

}

void HalfEdgeUtils::printMesh(Mesh * m) {
	vector<Facet*> & facets = m->facets;

	std::cout << "Mesh [ \n\t";
	for (int i = 0; i < facets.size();i++) {
		printFacet(facets[i]);
		std::cout << "\n\t";
	}
	std::cout << " ]";
}