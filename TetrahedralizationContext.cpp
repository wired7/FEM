#pragma once
#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include <thread>
#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"

#include "FPSCameraControls.h"

using namespace Geometry;

#define MIN_VOLUME 3

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
	
/*	thread t([=] {
		// compute tetrahedralization here
		tetrahedralizationReady = true;
	});
	t.detach();*/
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

	Geometry::Vertex * seed = vertices[shortestIndex];
	Geometry::Vertex * nearest = vertices[0];

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
	Geometry::Vertex* c = vertices[0];

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


	Geometry::Vertex* finalVertex = vertices[0];
	
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
	std::cout << "------------------------------------------------- " << std::endl;;

	for (int i = 0; i < mesh->facets.size();i++) {
		partitions.push_back(HalfEdgeUtils::makeFacetPartition(mesh->facets[i], positions, firstPartition));
		std::cout << "\n\nPARTITION " << i << "   ( "<<partitions[i].size()<<") : ";

		for (int j = 0; j < partitions[i].size();j++) {
			std::cout << partitions[i][j] << ", ";
		}
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

				if (HalfEdgeUtils::facetPointsTo(*testFacet, *vertex, positions) && HalfEdgeUtils::getFacetPointVolume(testFacet, vertex, positions)) {

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
	//	std::cout << "NEW Tetra: ";
	//	HalfEdgeUtils::printMesh(m);
	//	std::cout << std::endl;



		for (int i = 0; i < m->facets.size();i++) {
			Facet* f = m->facets[i];
			if (f != twinFacet) {
				openFacets.push_back(f);
			}
			partitions.push_back(HalfEdgeUtils::makeFacetPartition(f, positions, partitions[facet->externalIndex]));
		}
//		std::cout << "volume: " << HalfEdgeUtils::getTetraVolume(m, positions) << std::endl;;

	}



/*	for (int i = 0; i < totalMesh->facets.size();i++) {
		totalMesh->facets[i]->externalIndex = i;
	}
	for (int i = 0; i < totalMesh->halfEdges.size();i++) {
		totalMesh->halfEdges[i]->externalIndex = i;
	}
	*/
	std::cout << "size: " << totalMesh->facets.size() << std::endl;

}

bool TetrahedralizationContext::fillUpGaps() {
	vector<vector<Mesh*>> bfsResult = HalfEdgeUtils::BreadthFirstSearch(volume.meshes[0],100);
	int count = 0;
	for (int i = 0; i < bfsResult.size();i++) {
		vector<Mesh*> & level = bfsResult[i];
		count += level.size();

		std::cout << "\n\n\n#################################################################################"<< std::endl << std::endl;;
		std::cout << "level " << i << std::endl << std::endl;;
		for (int j = 0; j < level.size();j++) {
			HalfEdgeUtils::printMesh(level[j]);
			std::cout << "\n______________________________________________________________________\n";
		}
	}
	std::cout << "Total count: " << count << std::endl;;
	return false;
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
}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		tetrahedralizationReady = false;
	}

	if (length(cameras[0]->velocity) > 0) {
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}
}