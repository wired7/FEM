#pragma once
#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"

#include "FPSCameraControls.h"
#include <algorithm>

using namespace Geometry;

#define MIN_VOLUME 0.000000000

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, Graphics::DecoratedGraphicsObject* points, vector<vec3> &_points, FPSCamera* cam) : positions(_points)
{
	cameras.push_back(cam);

	setupGeometries();

	volume.totalMesh = new Mesh();
	Mesh* totalMesh = volume.totalMesh;
	totalMesh->volume = &volume;
	vector<Geometry::Vertex*>& vertices = totalMesh->vertices;

	vertices.resize(positions.size());
	for (int i = 0; i < vertices.size();i++) {
		vertices[i] = new Geometry::Vertex(i);
		usedVertices.push_back(false);
	}




	initialTetrahedralization();

	auto volumes = HalfEdgeUtils::getRenderableVolumesFromMesh(&volume, positions, refMan);
	geometries.push_back(volumes);

	auto edges = HalfEdgeUtils::getRenderableEdgesFromMesh(&volume, positions, refMan);
	geometries.push_back(edges);

	auto instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)points->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	int currentIndex = objectIDs[0];
	int currentManagedIndex = refMan->assignNewGUID();
	for (int i = 0; i < objectIDs.size(); i++)
	{
		if (objectIDs[i] != currentIndex)
		{
			currentIndex = objectIDs[i];
			currentManagedIndex = refMan->assignNewGUID();
		}

		objectIDs[i] = currentManagedIndex;
	}

	instanceIDs->extendedData = objectIDs;
	instanceIDs->updateBuffers();

	geometries.push_back(points);

	auto faces = HalfEdgeUtils::getRenderableFacetsFromMesh(&volume, positions);

	instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)faces->signatureLookup("INSTANCEID"));
	objectIDs = instanceIDs->extendedData;

	currentIndex = objectIDs[0];
	currentManagedIndex = refMan->assignNewGUID();
	for (int i = 0; i < objectIDs.size(); i++)
	{
		if (objectIDs[i] != currentIndex)
		{
			currentIndex = objectIDs[i];
			currentManagedIndex = refMan->assignNewGUID();
		}

		objectIDs[i] = currentManagedIndex;
	}

	instanceIDs->extendedData = objectIDs;
	instanceIDs->updateBuffers();
	geometries.push_back(faces);

	geometries.push_back(surface);

	setupPasses();
}

void TetrahedralizationContext::setupGeometries(void)
{
	refMan = new ReferenceManager();
}

void TetrahedralizationContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A"), ShaderProgramPipeline::getPipeline("EdgeA"), ShaderProgramPipeline::getPipeline("C"), ShaderProgramPipeline::getPipeline("D") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->addRenderableObjects(geometries[4], 0);
	gP->addRenderableObjects(geometries[1], 1);
	gP->addRenderableObjects(geometries[2], 2);
	gP->addRenderableObjects(geometries[3], 3);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[5], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void TetrahedralizationContext::initialTetrahedralization(void) {
	Mesh* totalMesh = volume.totalMesh;
	vector<Geometry::Vertex*> &vertices = totalMesh->vertices;
		
	vec3 centroid(0);
	for (int i = 0; i < positions.size(); i++) {
		centroid += positions[i];
	}
	centroid /= (float)positions.size();
	int shortestIndex = 0;
	float shortestDistance = distance(centroid,positions[0]);

	for (int i = 0; i < positions.size();i++) {
		if (distance(positions[i], centroid) < shortestDistance) {
			shortestDistance = distance(positions[i], centroid);
			shortestIndex = i;
		}
	}

	Geometry::Vertex * seed = vertices[0];
	Geometry::Vertex * nearest = vertices[1];

#pragma region first_halfedge
	vec3 posSeed  = positions[seed->externalIndex];
	vec3 posNearest = positions[nearest->externalIndex];

	shortestDistance = -1;
	// first first nearest neighbour;

	for (int i = 0; i < vertices.size();i++) {
		if (i!= seed->externalIndex) {
			Geometry::Vertex* nextVert = vertices[i];
			const vec3 & pos = positions[nextVert->externalIndex];
			float distance = glm::distance(posSeed, pos);

			if (distance < shortestDistance || shortestDistance == -1) {
				shortestDistance = distance;
				posNearest = pos;
				nearest = nextVert;
			}
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


	Geometry::Vertex* finalVertex = vertices[3];
	
	shortestDistance = HalfEdgeUtils::distanceToFacet(positions, *finalVertex, *facet);

	for (int i = 0; i < vertices.size();i++) {
		Geometry::Vertex& nextVertex = *vertices[i];

		if (!HalfEdgeUtils::containsVertex(nextVertex, *facet) && HalfEdgeUtils::getFacetPointVolume(facet,&nextVertex,positions) > MIN_VOLUME) {
			float distance = HalfEdgeUtils::distanceToFacet(positions, nextVertex, *facet);
			if (distance < shortestDistance) {
				shortestDistance = distance;
				finalVertex = &nextVertex;
			}
		}
	}

	Mesh* mesh = HalfEdgeUtils::constructTetrahedron(*finalVertex, *facet,vertices,positions);
	for (int i = 0; i < mesh->vertices.size();i++) {
		usedVertices[mesh->vertices[i]->externalIndex] = true;
	}
#pragma endregion

	openFacets.insert(openFacets.begin(), mesh->facets.begin(), mesh->facets.end());

	volume.addMesh(mesh);

	std::cout << "SEED Tetra: ";
	HalfEdgeUtils::printMesh(mesh);
	std::cout<< std::endl;



	// partition data
	vector<int> firstPartition;
	firstPartition.resize(vertices.size());
	for (int i = 0; i < vertices.size();i++) {
		firstPartition[i] = i;
	}


	for (int i = 0; i < mesh->facets.size();i++) {
		partitions.push_back(HalfEdgeUtils::makeFacetPartition(mesh->facets[i], positions, firstPartition));
	}

}

bool TetrahedralizationContext::addNextTetra() {
	Mesh* totalMesh = volume.totalMesh;

	vector<Geometry::Vertex*> &vertices = totalMesh->vertices;

	Geometry::Vertex* closest = vertices[0];
	Facet* facet = openFacets[0];
	int facetIndex = 0;
	float shortestDistance = -1;

	for (int i = 0; i < openFacets.size();i++) {

		Geometry::Facet* testFacet = openFacets[i];
		vector<int> &facetPartition = partitions[testFacet->externalIndex];

		for (int j = 0; j < facetPartition.size();j++) {
			int vertexIndex = facetPartition[j];
			Geometry::Vertex* vertex = totalMesh->vertices[vertexIndex];
			if (!usedVertices[vertex->externalIndex]) {

				if (HalfEdgeUtils::facetPointsTo(*testFacet, *vertex, positions) && HalfEdgeUtils::getFacetPointVolume(testFacet, vertex, positions) > MIN_VOLUME) {

					float distance = HalfEdgeUtils::distanceToFacet(positions, *vertex, *testFacet);
					if (distance < shortestDistance || shortestDistance == -1) {
						shortestDistance = distance;
						closest = vertex;
						facet = testFacet;
						facetIndex = i;
					}

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
		Mesh* m = HalfEdgeUtils::constructTetrahedron(*closest, *twinFacet,totalMesh->vertices, positions);
		
		for (int i = 0; i < m->vertices.size();i++) {
			usedVertices[m->vertices[i]->externalIndex] = true;
		}

		volume.addMesh(m);		
		std::cout << "Adding mesh" << m->internalIndex << " from facet " << facet->externalIndex << std::endl;

		std::cout << "NEW Tetra: ";
		HalfEdgeUtils::printMesh(m);
		std::cout << std::endl;



		for (int i = 0; i < m->facets.size();i++) {
			Facet* f = m->facets[i];
			if (f != twinFacet) {
				openFacets.push_back(f);
			}
			partitions.push_back(HalfEdgeUtils::makeFacetPartition(f, positions, partitions[facet->externalIndex]));
		}



	//	fillUpGaps(m);


	}
}

vector<Geometry::Mesh*> TetrahedralizationContext::fillUpGaps(Geometry::Mesh* mesh) {
	// use index 2 becasue we are lookinf for teatras 2 tetras away
	vector<Mesh*> newMeshes;
	vector<vector<Mesh*>> bfsResult = HalfEdgeUtils::BreadthFirstSearch(mesh,3);
	vector<Mesh*> tetras = bfsResult[1];
	tetras.insert(tetras.begin(),bfsResult[2].begin(), bfsResult[2].end());
	if (tetras.size() == 0) return newMeshes;





	for (int i = 0; i < mesh->facets.size();i++) {

		Facet* facet = mesh->facets[i];
		if (facet->twin != nullptr) continue;

		for (int j = 0;j < tetras.size();j++) {
			Mesh* tetra = tetras[j];

			for (int k = 0; k < tetra->facets.size();k++) {

				Facet* tetraFacet = tetra->facets[k];
				if (tetraFacet->twin != nullptr) continue;
				bool areFacingEachother = HalfEdgeUtils::facetsFaceEachother(facet, tetraFacet, positions);
				if (areFacingEachother) {
					vector<Geometry::Vertex*> vertices1 = HalfEdgeUtils::getFacetVertices(facet);
					vector<Geometry::Vertex*> vertices2 = HalfEdgeUtils::getFacetVertices(tetraFacet);
					vector<Geometry::Vertex*> facetIntersection = HalfEdgeUtils::getVertexIntersection(vertices1, vertices2);
	
					if (facetIntersection.size() == 2) {
						vector<Geometry::Vertex*> verts1 = HalfEdgeUtils::getVertexDifference(HalfEdgeUtils::getFacetVertices(facet), facetIntersection);
						vector<Geometry::Vertex*> verts2 = HalfEdgeUtils::getVertexDifference(HalfEdgeUtils::getFacetVertices(tetraFacet), facetIntersection);
						Facet* twin1 = HalfEdgeUtils::constructTwinFacet(facet);
						//Facet* twin2 = HalfEdgeUtils::constructTwinFacet(tetraFacet);

						Mesh* m = HalfEdgeUtils::constructTetrahedron(*verts2[0], *twin1, *tetraFacet, volume.totalMesh->vertices, positions);
						newMeshes.push_back( m );
						

						volume.addMesh(m);
						std::cout << "Filled mesh\n";
						HalfEdgeUtils::printMesh(m);
						cout << endl;

						
						vector<int> partitionUnion = HalfEdgeUtils::getPartitionUnion(partitions[facet->externalIndex], partitions[tetraFacet->externalIndex]);
						for (int p  = 0; p < m->facets.size();p++) {
							vector<int> newPartition = HalfEdgeUtils::makeFacetPartition(m->facets[p], positions, partitionUnion);
							partitions.push_back(newPartition);
						}
					}
				
				}

			
			}
			
		}


	}


	if (newMeshes.size() == 0) std::cout << "couldnt find anyting to fill" << std::endl;
	else {
		vector<Facet*>::iterator ite = remove_if(openFacets.begin(), openFacets.end(), [&](Facet* f)->bool {
			return (f->twin != nullptr);
		});
		openFacets.erase(ite, openFacets.end());
		for (int i = 0; i < newMeshes.size();i++) {
			vector<Facet*> & meshFacets = newMeshes[i]->facets;
			for (int j = 0; j < meshFacets.size();j++) {
				if (meshFacets[j]->twin == nullptr) {
					openFacets.push_back(meshFacets[j]);
				}
			}
		}

		std::cout << "\nSHOWING NEW MESHES: \n";
		for (int i = 0; i < newMeshes.size();i++) {
			std::cout << "\n\n";
			HalfEdgeUtils::printMesh(newMeshes[i]);
			std::cout << "\n\n";

		}



	}

	return newMeshes;
	
}
void TetrahedralizationContext::updateGeometries()
{
	glFinish();

	((GeometryPass*)passRootNode)->clearRenderableObjects(0);
	delete geometries[0];
	geometries[0] = HalfEdgeUtils::getRenderableVolumesFromMesh(&volume, positions, refMan);
	
	if (controller->volumeRendering)
	{
		((GeometryPass*)passRootNode)->addRenderableObjects(geometries[0], 0);
	}

	if (controller->surfaceRendering)
	{
		((GeometryPass*)passRootNode)->addRenderableObjects(geometries[4], 0);
	}

	((GeometryPass*)passRootNode)->clearRenderableObjects(1);
	delete geometries[1];
	geometries[1] = HalfEdgeUtils::getRenderableEdgesFromMesh(&volume, positions, refMan);

	if (controller->edgeRendering)
	{
		((GeometryPass*)passRootNode)->addRenderableObjects(geometries[1], 1);
	}

	((GeometryPass*)passRootNode)->clearRenderableObjects(3);
	delete geometries[3];
	geometries[3] = HalfEdgeUtils::getRenderableFacetsFromMesh(&volume, positions);

	if (controller->facetRendering)
	{
		((GeometryPass*)passRootNode)->addRenderableObjects(geometries[3], 3);
	}

	glFinish();

	dirty = true;
}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		controller->context->updateGeometries();
		tetrahedralizationReady = false;
	}

	if (length(cameras[0]->velocity) > 0) {
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}
}