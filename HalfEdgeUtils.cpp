#pragma once
#include "HalfEdgeUtils.h"
#include <iostream>
#include "LinearAlgebraUtils.h"
#include "GeometricalMeshObjects.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>

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

vector<vec3> HalfEdgeUtils::getVolumeVertices(Geometry::Mesh* mesh, const vector<vec3>& positions)
{
	vector<vec3> pos;

	for (int i = 0; i < mesh->vertices.size(); i++)
	{
		pos.push_back(positions[mesh->vertices[i]->externalIndex]);
	}

	return pos;
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
	const vector<Geometry::Vertex*> vertecies = getFacetVertices(&facet);
	float dist = 0;
	for (int i = 0; i < vertecies.size();i++) {
		const vec3 & facetPos = positions[vertecies[i]->externalIndex];
		dist += distance(vPos, facetPos);
	}

	return sqrtf(dist);
}

vec3 HalfEdgeUtils::getFacetCentroid(Geometry::Facet* facet, const vector<vec3>& positions, const mat4& parentTransform)
{
	vec3 centroid(0.0f);
	
	auto edges = getFacetHalfEdges(facet);

	for (int i = 0; i < edges.size(); i++)
	{
		centroid += vec3(parentTransform * vec4(positions[edges[i]->vertex->externalIndex], 1));
	}

	return centroid / (float)edges.size();
}

mat4 HalfEdgeUtils::getHalfEdgeTransform(Geometry::HalfEdge* halfEdge, const vector<vec3>& positions, const mat4& parentTransform, const vec3& centroid)
{
	vec3 point[2];
	point[0] = vec3(parentTransform * vec4(positions[halfEdge->start], 1));
	point[1] = vec3(parentTransform * vec4(positions[halfEdge->end], 1));
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
		scale(mat4(1.0f), vec3(1.001f)) *
		translate(mat4(1.0f), centroid) *
		scale(mat4(1.0f), vec3(0.7f)) *
		aroundCentroid;

	return transform;
}

Graphics::DecoratedGraphicsObject* HalfEdgeUtils::getRenderableFacetsFromMesh(Geometry::VolumetricMesh* meshes, const vector<vec3>& positions, const vector<mat4>& transforms)
{
	vector<GLuint> indices;
	vector<Graphics::Vertex> vertices;
	vector<GLuint> pickableIndices;
	int guid = 0;

	for (int j = 0; j < transforms.size(); j++)
	{
		for (int l = 0; l < meshes->meshes.size(); l++)
		{
			for (int i = 0; i < meshes->meshes[l]->facets.size(); i++)
			{
				auto facetVertices = HalfEdgeUtils::getFacetVertices(meshes->meshes[l]->facets[i]);
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->facets[i], positions, transforms[j]);
				vec3 normal = -normalize(cross(vec3(transforms[j] * vec4(positions[facetVertices[0]->externalIndex], 1)) - centroid,
					vec3(transforms[j] * vec4(positions[facetVertices[1]->externalIndex], 1)) - centroid));

				vertices.push_back(Graphics::Vertex(centroid, normal));

				guid++;
				pickableIndices.push_back(guid);
				int centroidIndex = vertices.size() - 1;

				for (int k = 0; k <= facetVertices.size(); k++)
				{
					vec3 pos = vec3(transforms[j] * vec4(positions[facetVertices[k % facetVertices.size()]->externalIndex], 1));

					pos = centroid + 0.9f * (pos - centroid);

					vertices.push_back(Graphics::Vertex(pos, normal));

					pickableIndices.push_back(guid);

					if (k > 0)
					{
						indices.push_back(centroidIndex);
						indices.push_back(centroidIndex + k - 1);
						indices.push_back(centroidIndex + k);
					}
				}

				indices.push_back(centroidIndex);
				indices.push_back(centroidIndex + facetVertices.size());
				indices.push_back(centroidIndex + 1);
			}
		}
	}

	auto meshObject = new Graphics::MeshObject(vertices, indices);

	auto pickable = new Graphics::ExtendedMeshObject<GLuint, GLuint>(meshObject, pickableIndices, "INSTANCEID");

	vector<GLbyte> selectedC;

	for (int i = 0; i < pickableIndices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::ExtendedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION");

	vector<mat4> parentTransforms;

	parentTransforms.push_back(scale(mat4(1.0f), vec3(1.001f)));

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM");

	return g;
}

Graphics::DecoratedGraphicsObject* HalfEdgeUtils::getRenderableEdgesFromMesh(Geometry::VolumetricMesh* meshes, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms)
{
	vector<mat4> transformC;
	vector<vec3> centroids; 
	vector<GLbyte> warningC;

	for (int j = 0; j < transforms.size(); j++)
	{
		for (int l = 0; l < meshes->meshes.size(); l++)
		{
			for (int i = 0; i < meshes->meshes[l]->facets.size(); i++)
			{
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->facets[i], positions, transforms[j]);
				auto edges = HalfEdgeUtils::getFacetHalfEdges(meshes->meshes[l]->facets[i]);

				for (int k = 0; k < edges.size(); k++)
				{
					warningC.push_back(0);
					transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], positions, transforms[j], centroid));
				}
			}

			for (int i = 0; i < meshes->meshes[l]->holes.size(); i++)
			{
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->holes[i], positions, transforms[j]);

				auto edges = HalfEdgeUtils::getFacetHalfEdges(meshes->meshes[l]->holes[i]);

				for (int k = 0; k < edges.size(); k++)
				{
					warningC.push_back(1);
					transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], positions, transforms[j], centroid));
				}
			}
		}
	}

	auto cylinder = new Arrow();

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(cylinder, transformC, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, transformC.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto highlightable = new InstancedMeshObject<GLbyte, GLbyte>(selectable, warningC, "WARNING", 1);

	return highlightable;
}

Graphics::DecoratedGraphicsObject* HalfEdgeUtils::getRenderableVolumesFromMesh(Geometry::VolumetricMesh* meshes, const vector<vec3>& positions, ReferenceManager* refMan, const vector<mat4>& transforms)
{
	vector<GLuint> indices;
	vector<Graphics::Vertex> vertices;
	vector<GLuint> pickableIndices;
	int guid = 0;

	for (int j = 0; j < transforms.size(); j++)
	{
		for (int l = 0; l < meshes->meshes.size(); l++)
		{
			guid++;
			vec3 volumeCentroid;
			auto volumeVertices = getVolumeVertices(meshes->meshes[l], positions);

			for (int i = 0; i < volumeVertices.size(); i++)
			{
				volumeCentroid += volumeVertices[i];
			}

			volumeCentroid /= (float)volumeVertices.size();

			mat4 volumeTransform = translate(mat4(1.0f), volumeCentroid) *
									scale(mat4(1.0f), vec3(0.99f)) *
									translate(mat4(1.0f), -volumeCentroid);

			for (int i = 0; i < meshes->meshes[l]->facets.size(); i++)
			{
				auto facetVertices = HalfEdgeUtils::getFacetVertices(meshes->meshes[l]->facets[i]);
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->facets[i], positions, transforms[j] * volumeTransform);
				vec3 normal = -normalize(cross(vec3(transforms[j] * vec4(positions[facetVertices[0]->externalIndex], 1)) - centroid,
					vec3(transforms[j] * vec4(positions[facetVertices[1]->externalIndex], 1)) - centroid));

				vertices.push_back(Graphics::Vertex(centroid, normal));

				pickableIndices.push_back(guid);
				int centroidIndex = vertices.size() - 1;

				for (int k = 0; k <= facetVertices.size(); k++)
				{
					vec3 pos = vec3(transforms[j] * volumeTransform * vec4(positions[facetVertices[k % facetVertices.size()]->externalIndex], 1));

					vertices.push_back(Graphics::Vertex(pos, normal));

					pickableIndices.push_back(guid);

					if (k > 0)
					{
						indices.push_back(centroidIndex);
						indices.push_back(centroidIndex + k - 1);
						indices.push_back(centroidIndex + k);
					}
				}

				indices.push_back(centroidIndex);
				indices.push_back(centroidIndex + facetVertices.size());
				indices.push_back(centroidIndex + 1);
			}
		}
	}

	auto meshObject = new Graphics::MeshObject(vertices, indices);

	auto pickable = new Graphics::ExtendedMeshObject<GLuint, GLuint>(meshObject, pickableIndices, "INSTANCEID");

	vector<GLbyte> selectedC;

	for (int i = 0; i < pickableIndices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::ExtendedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION");

	vector<mat4> parentTransforms;

	parentTransforms.push_back(mat4(1.0f));

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM");

	return g;
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
void HalfEdgeUtils::connectHalfEdges(std::vector<HalfEdge*> & halfedges) {
	 for (int i = 0; i < halfedges.size() - 1;i++) {
		 halfedges[i]->next = halfedges[i + 1];
	 }
	 halfedges[halfedges.size() - 1]->next = halfedges[0];
	 for (int i = halfedges.size()-1; i >1;i--) {
		 halfedges[i]->previous = halfedges[i - 1];
	 }
	 halfedges[0]->previous = halfedges[halfedges.size() - 1];

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