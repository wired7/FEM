#pragma once

#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include <thread>
#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"
using namespace Geometry;

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, vector<vec3> &_points, SphericalCamera* cam) : positions(_points)
{
	cameras.push_back(cam);
	geometries.push_back(surface);

	setupGeometries();

	totalMesh = new Mesh();
	vector<Geometry::Vertex*>& vertices = totalMesh->vertices;

	vertices.resize(positions.size());
	for (int i = 0; i < vertices.size();i++) {
		vertices[i] = new Geometry::Vertex(i);
		usedVertices.push_back(false);
	}

	initialTetrahedralization();

	setupPasses();
	
	thread t([=] {
		// compute tetrahedralization here
		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{

}

void TetrahedralizationContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("C") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void TetrahedralizationContext::initialTetrahedralization(void) {
	vector<Geometry::Vertex*> &vertices = totalMesh->vertices;
	vector<HalfEdge*> &halfedges = totalMesh->halfEdges;
	vector<Facet*> &facets = totalMesh->facets;

	Geometry::Vertex * seed = vertices[0];
	Geometry::Vertex * nearest = vertices[1];

#pragma region first_halfedge
	vec3 posSeed  = positions[seed->externalIndex];
	vec3 posNearest = positions[nearest->externalIndex];

	float shortestDistance = glm::distance(posSeed, posNearest);
	// first first nearest neighbour;

	for (int i = 1; i < vertices.size();i++) {
		Geometry::Vertex* nextVert = vertices[i];
		const vec3 & pos = positions[nextVert->externalIndex];
		float distance = glm::distance(posSeed, pos);

		if (distance < shortestDistance) {
			shortestDistance = distance;
			posNearest = pos;
			nearest = nextVert;
		}
	}
#pragma endregion

#pragma region first_facet

	Geometry::Vertex* a = seed;
	Geometry::Vertex* b = nearest;
	Geometry::Vertex* c = vertices[2];

	HalfEdge* HEab = new HalfEdge(seed, nearest);
	HalfEdge* HEbc;
	HalfEdge* HEca;

	shortestDistance = HalfEdgeUtils::distanceToHalfEdge(positions, *c, *HEab);

	for (int i = 0; i < vertices.size();i++) 
	{
		Geometry::Vertex & nextVert = *vertices[i];
		if (!HalfEdgeUtils::containsVertex(nextVert, *HEab)) {
			float distance = HalfEdgeUtils::distanceToHalfEdge(positions, nextVert, *HEab);
			if (distance == -1 || distance < shortestDistance)
			{
				shortestDistance = distance;
				c = &nextVert;
			}
		}
	}

	HEbc = new HalfEdge(b, c);
	HEca = new HalfEdge(c, a);

	vector<HalfEdge*> HEchain = { HEab,HEbc,HEca };
	
	HalfEdgeUtils::connectHalfEdges(HEchain);
	Facet * facet = new Facet(HEab);

#pragma endregion

#pragma region first_tetrahedron


	Geometry::Vertex* finalVertex = vertices[4];
	
	shortestDistance = HalfEdgeUtils::distanceToFacet(positions, *finalVertex, *facet);

	for (int i = 0; i < vertices.size();i++) {
		Geometry::Vertex& nextVertex = *vertices[i];

		if (!HalfEdgeUtils::containsVertex(nextVertex, *facet)) {
			float distance = HalfEdgeUtils::distanceToFacet(positions, nextVertex, *facet);
			if (distance < shortestDistance) {
				shortestDistance = distance;
				finalVertex = &nextVertex;
			}
		}
	}

	Mesh* mesh = HalfEdgeUtils::constructTetrahedron(*finalVertex, *facet,vertices);
	for (int i = 0; i < mesh->vertices.size();i++) {
		usedVertices[mesh->vertices[i]->externalIndex] = true;
	}
#pragma endregion



	facets.insert(facets.begin(), mesh->facets.begin(), mesh->facets.end());
	openFacets.insert(openFacets.begin(), mesh->facets.begin(), mesh->facets.end());
	halfedges.insert(halfedges.begin(), mesh->halfEdges.begin(), mesh->halfEdges.end());


	volume.meshes.push_back(mesh);


	std::cout << "SEED Tetra: ";
	HalfEdgeUtils::printMesh(mesh);
	std::cout<< std::endl;
}

bool TetrahedralizationContext::addNextFacet() {
	
	vector<Geometry::Vertex*> &vertices = totalMesh->vertices;

	Geometry::Vertex* closest = vertices[0];
	Facet* facet = openFacets[0];
	int facetIndex = 0;
	float shortestDistance = -1;
	for (int i = 0; i < vertices.size();i++) {
		Geometry::Vertex* vertex = vertices[i];
		if (!usedVertices[vertex->externalIndex]) {
			for (int j = 0; j < openFacets.size();j++) {
				float distance = HalfEdgeUtils::distanceToFacet(positions, *vertex, *openFacets[j]);
				if (distance < shortestDistance || shortestDistance == -1) {
					shortestDistance = distance;
					closest = vertex;
					facet = openFacets[j];
					facetIndex = j;
				}
			}
		}
	}

	if (shortestDistance == -1) {
		std::cout << "Could not add another face"<<std::endl;
		return true;
	}
	else {
		
		Facet* twinFacet = HalfEdgeUtils::constructTwinFacet(facet);
		openFacets.erase(openFacets.begin()+facetIndex);
		Mesh* m = HalfEdgeUtils::constructTetrahedron(*closest, *twinFacet,totalMesh->vertices);
		
		for (int i = 0; i < m->vertices.size();i++) {
			usedVertices[m->vertices[i]->externalIndex] = true;
		}

		volume.meshes.push_back(m);
		std::cout << "NEW Tetra: ";
		HalfEdgeUtils::printMesh(m);
		std::cout << std::endl;


		for (int i = 0; i < m->facets.size();i++) {
			Facet* f = m->facets[i];
			if (f != twinFacet) {
				openFacets.push_back(f);
			}
		}
	}




	
}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		tetrahedralizationReady = false;
	}
}