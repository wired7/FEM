#pragma once
#include "HalfEdgeUtils.h"
#include <iostream>
#include "LinearAlgebraUtils.h"
#include "GeometricalMeshObjects.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>
#include <algorithm>

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

vector<Geometry::Mesh*> HalfEdgeUtils::getNeighbouringMeshes(Geometry::Mesh* mesh) {
	vector<Mesh*> meshes;
	for (int i = 0; i < mesh->facets.size();i++) {
		if(mesh->facets[i]->twin!=nullptr &&mesh->facets[i]->twin->mesh!=nullptr)
			meshes.push_back(mesh->facets[i]->twin->mesh);
	}
	return meshes;
}

vector<Geometry::Mesh*> HalfEdgeUtils::getVertexMeshes(Geometry::Vertex* vertex)
{
	auto mesh = vertex->halfEdge->facet->mesh;
	auto neighbors = getNeighbouringMeshes(mesh);
	vector<Geometry::Mesh*> outputMeshes;

	for (int i = 0; i < neighbors.size(); i++)
	{
		if (containsVertex(*vertex, *(neighbors[i])))
		{
			outputMeshes.push_back(neighbors[i]);
		}
	}

	return outputMeshes;
}

vector<Geometry::Mesh*> HalfEdgeUtils::getEdgeMeshes(pair<Geometry::Vertex*, Geometry::Vertex*> edge)
{
	auto mesh = edge.first->halfEdge->facet->mesh;
	auto neighbors = getNeighbouringMeshes(mesh);
	vector<Geometry::Mesh*> outputMeshes;

	for (int i = 0; i < neighbors.size(); i++)
	{
		if (containsVertex(*(edge.first), *(neighbors[i])) && containsVertex(*(edge.second), *(neighbors[i])))
		{
			// I know. No time for elegant solutions. Sorry
			for (int j = 0; j < neighbors[i]->halfEdges.size(); j++)
			{
				if (neighbors[i]->halfEdges[j]->start == edge.first->externalIndex && neighbors[i]->halfEdges[j]->end == edge.second->externalIndex || 
					neighbors[i]->halfEdges[j]->end == edge.first->externalIndex && neighbors[i]->halfEdges[j]->start == edge.second->externalIndex)
				{
					outputMeshes.push_back(neighbors[i]);
					break;
				}
			}
		}
	}

	return outputMeshes;
}

vector<vector<Geometry::Mesh*>> HalfEdgeUtils::BreadthFirstSearch(Geometry::Mesh* mesh, int depth) {

	vector<vector<Mesh*>> result(depth+1);
	result[0].push_back(mesh);
	vector<bool> visited(mesh->volume->meshes.size(),false);
	visited[mesh->internalIndex] = true;

	for (int i = 1; i < depth;i++) {

		vector<Mesh*> & previous = result[i - 1];
		vector<Mesh*> & meshes = result[i];

		for (int j = 0; j < previous.size();j++) {

			Mesh* currentMesh = previous[j];
			vector<Mesh*> currentMeshNeighbours = getNeighbouringMeshes(currentMesh);
		
			for (int k = 0; k < currentMeshNeighbours.size();k++) {

				if (!visited[currentMeshNeighbours[k]->internalIndex]) {

					visited[currentMeshNeighbours[k]->internalIndex] = true;
					meshes.push_back(currentMeshNeighbours[k]);

				}
			}

		}
	}

	return result;

}

float HalfEdgeUtils::distanceToHalfEdge(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::HalfEdge & halfedge) {

	const vec3 & vPos = positions[vertex.externalIndex];
	const vec3 & hPos1 = positions[halfedge.start];
	const vec3 & hPos2 = positions[halfedge.end];

	float f1 = glm::distance(hPos1, vPos);
	float f2 = glm::distance(hPos2, vPos);

	return f1*f1 + f2*f2;
}

float HalfEdgeUtils::distanceToFacet(vector<vec3> & positions, Geometry::Vertex & vertex, Geometry::Facet & facet) {

	const vec3 & vPos = positions[vertex.externalIndex];
	const vector<Geometry::Vertex*> vertecies = getFacetVertices(&facet);
	float dist = 0;
	for (int i = 0; i < vertecies.size();i++) {
		const vec3  facetPos = positions[vertecies[i]->externalIndex];
		float d = distance(vPos, facetPos);
		d = d*d;
		dist += d;
	}

	return dist;
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

vec3 HalfEdgeUtils::getMeshCentroid(Geometry::Mesh* mesh, const vector<vec3>& positions) {
	vector<vec3> vertexPositions = getVolumeVertices(mesh, positions);

	glm::vec3 center(0);
	for (int i = 0; i < vertexPositions.size();i++) {
		center += vertexPositions[i];
	}
	center /= (float)vertexPositions.size();

	return center;
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
float HalfEdgeUtils::getFacetPointVolume(Geometry::Facet* facet, Geometry::Vertex* vertex, vector<vec3> & positions) {
	vec3 facetNorm = getFacetDirection(facet, positions);
	Geometry::Vertex* tempVert = facet->halfEdge->vertex;
	vec3 pos = positions[vertex->externalIndex];
	vec3 pos2 = positions[tempVert->externalIndex];

	return abs(dot(facetNorm, pos - pos2));

}

float HalfEdgeUtils::getTetraVolume(Geometry::Mesh* mesh, vector<vec3> & positions) {

	Facet* facet = mesh->facets[0];
	HalfEdge* nextEdge = facet->halfEdge->twin->next;
	vec3 facetNorm = getFacetDirection(facet, positions);
	vec3 otherNorm = positions[nextEdge->end] - positions[nextEdge->start];
	return abs(dot(facetNorm, otherNorm));
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
			vec3 volumeCentroid;
			auto volumeVertices = getVolumeVertices(meshes->meshes[l], positions);

			for (int i = 0; i < volumeVertices.size(); i++)
			{
				volumeCentroid += vec3(transforms[j] * vec4(volumeVertices[i], 1.0f));
			}

			volumeCentroid /= (float)volumeVertices.size();

			mat4 volumeTransform = translate(mat4(1.0f), volumeCentroid) *
				scale(mat4(1.0f), vec3(0.8f)) *
				translate(mat4(1.0f), -volumeCentroid);

			for (int i = 0; i < meshes->meshes[l]->facets.size(); i++)
			{
				auto facetVertices = HalfEdgeUtils::getFacetVertices(meshes->meshes[l]->facets[i]);
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->facets[i], positions, transforms[j] * volumeTransform);
				vec3 normal = -normalize(cross(vec3(transforms[j] * volumeTransform * vec4(positions[facetVertices[0]->externalIndex], 1)) - centroid,
					vec3(transforms[j] * volumeTransform * vec4(positions[facetVertices[1]->externalIndex], 1)) - centroid));

				vertices.push_back(Graphics::Vertex(centroid, normal));

				guid++;
				pickableIndices.push_back(guid);
				int centroidIndex = vertices.size() - 1;

				for (int k = 0; k <= facetVertices.size(); k++)
				{
					vec3 pos = vec3(transforms[j] * volumeTransform * vec4(positions[facetVertices[k % facetVertices.size()]->externalIndex], 1));

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
			vec3 volumeCentroid;
			auto volumeVertices = getVolumeVertices(meshes->meshes[l], positions);

			for (int i = 0; i < volumeVertices.size(); i++)
			{
				volumeCentroid += vec3(transforms[j] * vec4(volumeVertices[i], 1.0f));
			}

			volumeCentroid /= (float)volumeVertices.size();

			mat4 volumeTransform = translate(mat4(1.0f), volumeCentroid) *
				scale(mat4(1.0f), vec3(0.8f)) *
				translate(mat4(1.0f), -volumeCentroid);

			for (int i = 0; i < meshes->meshes[l]->facets.size(); i++)
			{
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->facets[i], positions, transforms[j] * volumeTransform);
				auto edges = HalfEdgeUtils::getFacetHalfEdges(meshes->meshes[l]->facets[i]);

				for (int k = 0; k < edges.size(); k++)
				{
					warningC.push_back(0);
					transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], positions, transforms[j] * volumeTransform, centroid));
				}
			}

			for (int i = 0; i < meshes->meshes[l]->holes.size(); i++)
			{
				vec3 centroid = HalfEdgeUtils::getFacetCentroid(meshes->meshes[l]->holes[i], positions, transforms[j] * volumeTransform);

				auto edges = HalfEdgeUtils::getFacetHalfEdges(meshes->meshes[l]->holes[i]);

				for (int k = 0; k < edges.size(); k++)
				{
					warningC.push_back(1);
					transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], positions, transforms[j] * volumeTransform, centroid));
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
				volumeCentroid += vec3(transforms[j] * vec4(volumeVertices[i], 1.0f));
			}

			volumeCentroid /= (float)volumeVertices.size();

			mat4 volumeTransform = translate(mat4(1.0f), volumeCentroid) *
				scale(mat4(1.0f), vec3(0.8f)) *
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
		pickableIndices.push_back(guid);
		pickableIndices.push_back(guid);

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

bool HalfEdgeUtils::containsVertex(Geometry::Vertex & vertex, Mesh & mesh) {
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

bool HalfEdgeUtils::findHalfEdgeTwin(Geometry::Facet* facet, Geometry::HalfEdge* halfedge) {

	vector<HalfEdge*> facetHalfedges = getFacetHalfEdges(facet);

	for (int i = 0; i < facetHalfedges.size();i++) {
		HalfEdge* potential = facetHalfedges[i];
		if (potential->end == halfedge->start && potential->start == halfedge->end) {
			potential->twin = halfedge;
			halfedge->twin = potential;
			return true;
		}
	}
	return false;
}

bool HalfEdgeUtils::areTwins(Facet* facet1, Facet* facet2) {
	vector<Geometry::Vertex*> vertexIntersection = getVertexIntersection(facet1, facet2);
	if (vertexIntersection.size() == getFacetVertices(facet1).size() && vertexIntersection.size() == getFacetVertices(facet2).size())
		return true;
	else
		return false;

}

// returns facet of mesh1 that has a twin in mesh2
Facet* HalfEdgeUtils::findFacetWithTwin(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2) {
	for(int i = 0; i < mesh1->facets.size();i++)
	{
		for (int j = 0; j < mesh2->facets.size();j++) {
			if (areTwins(mesh1->facets[i], mesh2->facets[j])) {
				return mesh1->facets[i];
			}
		}
	}
	return nullptr;
}

bool HalfEdgeUtils::facetPointsTo(Geometry::Facet & facet, Geometry::Vertex & vertex, vector<vec3> & positions) {
	mat4 mat(1.0f);
	vec3 facetDirection = HalfEdgeUtils::getFacetDirection(&facet, positions);
	if (facet.mesh->isOutsideOrientated)
		facetDirection *= -1.0f;

	vec3 facetCentroid = getFacetCentroid(&facet, positions, mat);
	vec3 vertexPos = positions[vertex.externalIndex];
	vec3 vertexDirection = facetCentroid - vertexPos;

	return dot(vertexDirection, facetDirection) > 0;

}

// doesnt correct orientation (multiply by -1.0f if facet's mesh is not outsideOrientated
vec3 HalfEdgeUtils::getFacetDirection(Geometry::Facet* facet, const vector<vec3> & positions) {

	HalfEdge* halfedge = getFacetHalfEdges(facet)[0];

	vec3 p1 = positions[halfedge->start];
	vec3 p2 = positions[halfedge->end];
	vec3 p3 = positions[halfedge->next->end];

	return normalize(cross(p1 - p2, p3 - p2));

}
vec3 HalfEdgeUtils::getCorrectedFacetDirection(Geometry::Facet* facet, const vector<vec3> & positions) {

	HalfEdge* halfedge = getFacetHalfEdges(facet)[0];

	vec3 p1 = positions[halfedge->start];
	vec3 p2 = positions[halfedge->end];
	vec3 p3 = positions[halfedge->next->end];

	vec3 res = cross(p1 - p2, p3 - p2);
	if (!facet->mesh->isOutsideOrientated) res *= -1.0f;

	return res;

}
bool HalfEdgeUtils::getOrientation(Geometry::Mesh* mesh, const vector<vec3>& positions) {
	glm::mat4 mat(1.0f);

	Facet* facet = mesh->facets[0];
	vec3 meshCenter = getMeshCentroid(mesh, positions);
	vec3 facetCentroid = getFacetCentroid(facet, positions, mat);

	vec3 outDirection = facetCentroid - meshCenter;
	vec3 facetDirection = getFacetDirection(facet, positions);

	return dot(outDirection, facetDirection) > 0;
}


bool HalfEdgeUtils::facetsFaceEachother(Geometry::Facet* facet1, Geometry::Facet* facet2, vector<vec3> &positions) {
	vec3 v1 = getCorrectedFacetDirection(facet1,positions);
	vec3 v2 = getCorrectedFacetDirection(facet2, positions);

	vec3 p1 = getFacetCentroid(facet1, positions);
	vec3 p2 = getFacetCentroid(facet2, positions);

	if (dot(v1, p2 - p1) <0) return false;
	else if (dot(v2, p1 - p2) <0) return false;
	else return true;


}

vector<int> HalfEdgeUtils::makeFacetPartition(Geometry::Facet * facet, vector<vec3> & positions, vector<int> currentPartition) {

	vector<int> partition;

	vec3 facetDiretion = getFacetDirection(facet, positions);
	if (!facet->mesh->isOutsideOrientated)
		facetDiretion *= -1;

	vec3 facetCentroid = getFacetCentroid(facet, positions);

	for (int i = 0; i < currentPartition.size();i++) {
		int index = currentPartition[i];
		vec3 direction = positions[index] - facetCentroid;
		if (dot(direction, facetDiretion) > 0) {
			partition.push_back(index);
		}
	}
	return partition;
}

// the partitions should already be sorted, but if not then add sort to this
vector<int> HalfEdgeUtils::getPartitionIntersection(vector<int> & partition1, vector<int> & partition2) {
	
	vector<int> intersection;

	int cursor1 = 0;
	int cursor2 = 0;

	while (cursor1 != partition1.size()-1 && cursor2 != partition2.size()-1) {
		if (partition1[cursor1] < partition2[cursor2]) cursor1++;
		else if (partition2[cursor2] < partition1[cursor1]) cursor2++;
		else {
			intersection.push_back(partition1[cursor1]);
			cursor1++;
			cursor2++;
		}
	}
	return intersection;
}
vector<int> HalfEdgeUtils::getPartitionUnion(vector<int> & partition1, vector<int> & partition2) {
	vector<int> partitionUnion(partition2.size() + partition1.size());
	vector<int> p1 = partition1;
	vector<int> p2 = partition2;

	vector<int>::iterator it = set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), partitionUnion.begin());
	partitionUnion.resize(it - partitionUnion.begin());
	return partitionUnion;
}

vector<Geometry::Vertex*> HalfEdgeUtils::getVertexIntersection(vector<Geometry::Vertex*>& vertices1, vector<Geometry::Vertex*>& vertices2) {

	auto sortFunc = [&](Geometry::Vertex* v1, Geometry::Vertex* v2) ->bool
	{
		return v1->externalIndex < v2->externalIndex;
	};
	vector<Geometry::Vertex*> verts1 = vertices1;
	vector<Geometry::Vertex*> verts2 = vertices2;

	sort(verts1.begin(), verts1.end(), sortFunc);
	sort(verts2.begin(), verts2.end(), sortFunc);

	vector<Geometry::Vertex*> intersection;

	vector<Geometry::Vertex*>::iterator v1Ite = verts1.begin();
	vector<Geometry::Vertex*>::iterator v2Ite = verts2.begin();

	
	while (v1Ite != verts1.end() && v2Ite != verts2.end())
	{
		if ((*v1Ite)->externalIndex<(*v2Ite)->externalIndex) ++v1Ite;
		else if ((*v2Ite)->externalIndex<(*v1Ite)->externalIndex) ++v2Ite;
		else {
			intersection.push_back(*v1Ite);
			++v1Ite; ++v2Ite;
		}
	}

	return intersection;

}

vector<Geometry::Vertex*> HalfEdgeUtils::getVertexDifference(vector<Geometry::Vertex*> &vertices1, vector<Geometry::Vertex*> & vertices2) {
	auto sortFunc = [&](Geometry::Vertex* v1, Geometry::Vertex* v2) ->bool
	{
		return v1->externalIndex < v2->externalIndex;
	};
	vector<Geometry::Vertex*> verts1 = vertices1;
	vector<Geometry::Vertex*> verts2 = vertices2;

	sort(verts1.begin(), verts1.end(), sortFunc);
	sort(verts2.begin(), verts2.end(), sortFunc);

	vector<Geometry::Vertex*> difference;

	vector<Geometry::Vertex*>::iterator v1Ite = verts1.begin();
	vector<Geometry::Vertex*>::iterator v2Ite = verts2.begin();


	while (v1Ite != verts1.end() && v2Ite != verts2.end())
	{
		if ((*v1Ite)->externalIndex < (*v2Ite) ->externalIndex) 
		{ 
			difference.push_back( *v1Ite ); 
			++v1Ite; 
		}
		else if ((*v2Ite)->externalIndex <(*v1Ite)->externalIndex)
			++v2Ite;
		else {
			++v1Ite; 
			++v2Ite; 
		}
	}
	difference.insert(difference.end(), v1Ite, verts1.end());

	return difference;
}

vector<Geometry::Vertex*> HalfEdgeUtils::getVertexIntersection(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2) {
	return getVertexIntersection(mesh1->vertices, mesh2->vertices);
}

vector<Geometry::Vertex*> HalfEdgeUtils::getVertexIntersection(Geometry::Facet* facet1, Geometry::Facet* facet2) {
	return getVertexIntersection(getFacetVertices(facet1), getFacetVertices(facet2));
}





// this connects halfedges in the orther they are in the vector... it will not ensure that they connected correctly
vector<Geometry::Vertex*> HalfEdgeUtils::connectHalfEdges(std::vector<HalfEdge*> & halfedges) {
	vector<Geometry::Vertex*> vertices;
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



	for (int i = 0; i < facets.size();i++) {

		vector<HalfEdge*> facetHalfedges = getFacetHalfEdges(facets[i]);
		halfedges.insert(halfedges.end(), facetHalfedges.begin(), facetHalfedges.end());

		for (int j = 0; j < facetHalfedges.size();j++) {
			HalfEdge* halfedge = facetHalfedges[j];
			if (halfedge->twin == nullptr) {
				for (int k = 0; k < facets.size();k++) {
					if (k != i) {
						if (findHalfEdgeTwin(facets[k], halfedge)) break;
					}
				}
			}
		
		}
	}


	return halfedges;
}


Facet* HalfEdgeUtils::constructFacet(vector<Geometry::Vertex*> &vertices) {

	vector<HalfEdge*> halfedges;
	for (int i = 0; i < vertices.size()-1;i++) {
		halfedges.push_back(new HalfEdge(vertices[i], vertices[i+1]));
	}
	halfedges.push_back(new HalfEdge(vertices[vertices.size()-1], vertices[0] ));

	connectHalfEdges(halfedges);

	for (int i = 0; i < halfedges.size();i++) {
		halfedges[0]->internalIndex = i;
	}
	return new Facet(halfedges[0]);
}

Facet* HalfEdgeUtils::constructTwinFacet(Facet* facet) {
	
	vector<Geometry::Vertex*> vertices = getFacetVertices(facet);
	vector<Geometry::Vertex*> reverse(vertices.size());

	for (int i = reverse.size() - 1; i >= 0; i--) {
		reverse[reverse.size() - (i + 1)] = vertices[i];
	}

	Facet* twin = constructFacet(reverse);

	twin->twin = facet;
	facet->twin = twin;
	return twin;
}


Geometry::Mesh* HalfEdgeUtils::constructTetrahedron(Geometry::Vertex & vertex, Geometry::Facet & facet, vector<Geometry::Vertex*> &vertices, const vector<vec3>& positions) {


	vector<Facet*> facets;
	facets.push_back(&facet);
	HalfEdge * start = facet.halfEdge;
	HalfEdge * next = start;
	
	do {
		//opposite orientation
		
		Geometry::Vertex* a = next->vertex;
		Geometry::Vertex* b = next->previous->vertex;
		Geometry::Vertex* c = &vertex;

		vector<Geometry::Vertex*> vertices = { a,b,c };
		facets.push_back(constructFacet(vertices));
		next = next->next;
	} while (next != start);
	

	Mesh* mesh = new Mesh();
	mesh->halfEdges = connectFacets(facets, vertices.size());
	mesh->facets = facets;
	for (int i = 0; i < facets.size();i++) {
		facets[i]->mesh = mesh;
		facets[i]->internalIndex = i;
	}
	for (int i = 0; i < vertices.size();i++) {
		if (containsVertex(*vertices[i],*mesh)) {
			mesh->vertices.push_back(vertices[i]);
		}
	}

	mesh->isOutsideOrientated = getOrientation(mesh, positions );
	return mesh;
}

Geometry::Mesh* HalfEdgeUtils::constructTetrahedron(Geometry::Vertex & vertex, Geometry::Facet  & facet1, Geometry::Facet & facet2, vector<Geometry::Vertex*> & vertices, const vector<vec3>& positions) {
	Mesh* mesh = constructTetrahedron(vertex, facet1, vertices, positions);
	for (int i = 0; i < mesh->facets.size();i++){
		Facet& facet = *mesh->facets[i];
		if (facet.twin != nullptr) continue;
		if (areTwins(&facet, &facet2)) {
			facet.twin = &facet2;
			facet2.twin = &facet;
			cout << "\nmade twins " << facet2.externalIndex << " and " << facet.externalIndex << std::endl;

			break;
		}
	}
	return mesh;
}







//Printouts
void HalfEdgeUtils::printEdge(HalfEdge* he) {
	std::cout <<"HE "<< he->internalIndex << " : " << he->externalIndex;
	std::cout << " -> ( " << he->start << ", " << he->end<<" ) ";
	

}

void HalfEdgeUtils::printFacet(Facet * facet) {

	vector<HalfEdge*> halfedges = getFacetHalfEdges(facet);
	std::cout << "FACET   " << facet->externalIndex;
	if (facet->twin != nullptr) {
		cout<< " : " << facet->twin->externalIndex;
	}
	cout << "    { ";
	for (int i = 0; i < halfedges.size();i++) {
		printEdge(halfedges[i]);
		std::cout << ", ";
	}
	std::cout << " }";

}

void HalfEdgeUtils::printMesh(Mesh * m) {
	vector<Facet*> & facets = m->facets;

	std::cout << "Mesh "<<m->internalIndex;
	if (m->isOutsideOrientated)
		std::cout << " ( points Outwards )";
	else
		std::cout << " ( Points Inwards ) ";

	std::cout << " { \n\t";

	for (int i = 0; i < facets.size();i++) {
		printFacet(facets[i]);
		std::cout << "\n\t";
	}

	std::cout << "       }";
}