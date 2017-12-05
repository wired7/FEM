#pragma once
#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include "HalfSimplices.h"
#include "HalfEdgeUtils.h"

#include "FPSCameraControls.h"
#include "VoronoiDiagramUtils.h"
#include "MeshGraph.h"
#include <algorithm>    // std::sort
#include "ImplicitGeometry.h"
#include <omp.h>
using namespace Geometry;

#define MIN_VOLUME 0.1

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface, Graphics::DecoratedGraphicsObject* points, vector<vec3> &_points, FPSCamera* cam) : positions(_points)
{
	sweepIndex = 0;

	currentInd = 0;
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
		volume.vertexMeshMapping.push_back(vector<Mesh*>(0));

	}


	auto sortFunc = [&](vec3 & v1, vec3 v2) -> bool{
		return v1.y < v2.y;
	};

	std::sort(_points.begin(), _points.end(), sortFunc);


	vector<pair<int, vec3>> mappedPositions;

	for (int i = 0; i < _points.size(); i++)
	{
		mappedPositions.push_back(pair<int, vec3>(i, _points[i]));
	}

	vector<function<bool(pair<int, vec3>, pair<int, vec3>)>> functors;
	vector<vector<pair<int, vec3>>> indexedByN {mappedPositions, mappedPositions, mappedPositions};
	vector <vector<int>> mapFromOriginalToSorted(3);
	

	for (int i = 0; i < 3; i++)
	{
		functors.push_back([&](pair<int, vec3> a, pair<int, vec3> b) { return a.second[i] < b.second[i]; });
		sort(indexedByN[i].begin(), indexedByN[i].end(), functors[i]);

		mapFromOriginalToSorted[i].resize(_points.size());
		for (int j = 0; j < indexedByN[i].size(); j++)
		{
			mapFromOriginalToSorted[i][indexedByN[i][j].first] = j;
		}
	}

	std::cout << "BEGIN" << endl;
#undef max
#undef min
	int windowSize = 10;
	for (int i = 0; i < _points.size(); i++)
	{
		vector<int> closeSpaceIndices;

		int iMappedToX = mapFromOriginalToSorted[0][i];
		int iMappedToY = mapFromOriginalToSorted[1][i];
		int iMappedToZ = mapFromOriginalToSorted[2][i];
		for (int j = std::max(iMappedToX - windowSize, 0); j <= std::min(iMappedToX + windowSize, (int)indexedByN[0].size() - 1); j++)
		{
			if (j == iMappedToX)
			{
				continue;
			}

			int jMappedToY = mapFromOriginalToSorted[1][indexedByN[0][j].first];
			int jMappedToZ = mapFromOriginalToSorted[2][indexedByN[0][j].first];

			if (jMappedToY < iMappedToY - windowSize || jMappedToY > iMappedToY + windowSize || jMappedToZ < iMappedToZ - windowSize || jMappedToZ > iMappedToZ > iMappedToZ + windowSize)
			{
				closeSpaceIndices.push_back(indexedByN[0][j].first);
			}
		}

//		cout << closeSpaceIndices.size() << endl;
		for (int j = 0; j < closeSpaceIndices.size(); j++)
		{
			for (int k = j + 1; k < closeSpaceIndices.size(); k++)
			{
				for (int l = k + 1; l < closeSpaceIndices.size(); l++)
				{
					for (int m = l + 1; m < closeSpaceIndices.size(); m++)
					{
						if (closeSpaceIndices[m] == closeSpaceIndices[l] || closeSpaceIndices[m] == closeSpaceIndices[k] || closeSpaceIndices[m] == closeSpaceIndices[j])
						{
							continue;
						}

						vec3 tPoints[4] = { _points[closeSpaceIndices[j]], _points[closeSpaceIndices[k]], _points[closeSpaceIndices[l]], _points[closeSpaceIndices[m]] };
						auto sphere = VoronoiDiagramUtils::getCircumsphere(tPoints);
						bool success = true;

						vec3 minBounds = sphere.center - sphere.radius;
						vec3 maxBounds = sphere.center + sphere.radius;

						for (int a = 0; a < _points.size(); a++)
						{
							if (a == j || a == k || a == l || a == m)
							{
								continue;
							}

							if (length(sphere.center - _points[a]) < sphere.radius)
							{
								success = false;
								break;
							}
						}

						if (success)
						{
							tetrahedra.push_back(ivec4(j, k, l, m));
						}
					}
				}
			}
		}
	}
	std::cout << tetrahedra.size() << endl;
/*	for (int i = 0; i < tetrahedra.size(); i++)
	{
		for (int c = 0; c < 4; c++)
		{
			cout << tetrahedra[i][c] << " ";
		}
		cout << endl;
	}*/
/*	auto meshGraph = new MeshGraph();
	for (int i = 0; i < _points.size(); i++)
	{
		meshGraph->nodes.push_back(new VertexNode(new Geometry::Vertex(i) ));
	}

	for (int i = 0; i < tetrahedra.size(); i++)
	{
		for (int j = 0; j < tetrahedra[i].length() - 1; j++)
		{
			for (int k = j + 1; k < tetrahedra[i].length(); k++)
			{
				meshGraph->nodes[j]->neighbors.push_back(meshGraph->nodes[k]);
				meshGraph->nodes[k]->neighbors.push_back(meshGraph->nodes[j]);
			}
		}
	}

	*/
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



void TetrahedralizationContext::sweepInit() {

	if ( positions.size()>=4) {
		Geometry::Mesh* totalMesh = volume.totalMesh;
		vector<Geometry::Vertex*> &vertices = totalMesh->vertices;
		vector<Geometry::Vertex*> tetraVerts(0);



		Geometry::Vertex * v1 = vertices[sweepIndex++];
		Geometry::Vertex * v2 = vertices[sweepIndex++];
		Geometry::Vertex * v3 = vertices[sweepIndex++];
		Geometry::Vertex * v4 = vertices[sweepIndex++];



		Geometry::Mesh* mesh = HalfEdgeUtils::constructTetrahedron(v1,v2,v3,v4,vertices,positions);
		HalfEdgeUtils::addMeshToVolume(mesh,&volume);
		for (int i = 0; i < mesh->facets.size();i++) {
			openFacets.push_back(mesh->facets[i]);
			triangles.push_back(HalfEdgeUtils::facetToTriangle(mesh->facets[i], positions));
			triangleIndecies.push_back(mesh->facets[i]->externalIndex);
		}
	}
	else
		std::cout << "Couldnt generate another tetrahedron" << std::endl;

}

void TetrahedralizationContext::sweep() {
	vector<Mesh*> newMeshes;

	if (sweepIndex > 3) {
		Geometry::Vertex* nextSweep = volume.totalMesh->vertices[sweepIndex];
		vector<Mesh*> newMeshes;
		vector<Geometry::Facet*> facets;
		for (int i = 0; i < openFacets.size();i++) {

			Geometry::Facet* facet = openFacets[i];

				bool isGood = HalfEdgeUtils::vertexSeesFacet(nextSweep, facet, triangles, positions);
				if (isGood) {
					std::cout << "Can see facet " << facet->externalIndex << std::endl;
					Facet* twin = HalfEdgeUtils::constructTwinFacet(facet);
					Mesh*m = HalfEdgeUtils::constructTetrahedron(*nextSweep, *twin, volume.totalMesh->vertices, positions);
					newMeshes.push_back(m);
					for (int x = 0; x < volume.meshes.size();x++) {
						pair<Facet*,Facet*> facetPair = HalfEdgeUtils::findFacetWithTwin(m, volume.meshes[x]);
						if (facetPair.first != nullptr && facetPair.second != nullptr) {
							facetPair.first->twin = facetPair.second;
							facetPair.second->twin = facetPair.first;
							std::cout << "Found pair" << std::endl;
						}
					}
					newMeshes.push_back(m);
					std::cout << "New mesh!" << std::endl;
					for (int x = 0; x < m->facets.size();x++) {
						triangles.push_back(HalfEdgeUtils::facetToTriangle(m->facets[x], positions));
					}
				}
				else
					std::cout << "Cannot see facet " << facet->externalIndex << std::endl;
		}

		for (int i = 0; i < newMeshes.size();i++) {
			HalfEdgeUtils::addMeshToVolume(newMeshes[i], &volume);
			for (int j = 0; j < newMeshes[j]->facets.size();j++) {
				if (newMeshes[i]->facets[j]->twin == nullptr)
					openFacets.push_back(newMeshes[i]->facets[j]);
			}

			vector<Facet*>::iterator ite = remove_if(openFacets.begin(), openFacets.end(), [&](Facet* f)->bool {
				return (f->twin != nullptr);
			});
			openFacets.erase(ite, openFacets.end());
		}
	}
	sweepIndex++;
	std::cout << "could not generate mesh!" << std::endl;
	std::cout << sweepIndex;
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
	HalfEdgeUtils::addMeshToVolume(mesh, &volume);

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


bool TetrahedralizationContext::addTetraFromGraph() {

	if (currentInd < tetrahedra.size()) {
		Mesh* totalMesh = volume.totalMesh;
		vector<Geometry::Vertex*> &vertices = totalMesh->vertices;



		Geometry::Vertex * v1 = vertices[tetrahedra[currentInd].x];
		Geometry::Vertex * v2 = vertices[tetrahedra[currentInd].y];
		Geometry::Vertex * v3 = vertices[tetrahedra[currentInd].z];
		Geometry::Vertex * v4 = vertices[tetrahedra[currentInd].w];


		HalfEdge* HEab = new HalfEdge(v1, v2);
		HalfEdge* HEbc = new HalfEdge(v2, v3);
		HalfEdge* HEca = new HalfEdge(v3, v1);

		vector<HalfEdge*> HEchain = { HEab,HEbc,HEca };

		HalfEdgeUtils::connectHalfEdges(HEchain);
		Facet * facet = new Facet(HEab);


		Mesh* mesh = HalfEdgeUtils::constructTetrahedron(*v4, *facet, vertices, positions);


		HalfEdgeUtils::addMeshToVolume(mesh, &volume);

		//	std::cout << "Tetra: ";
		//	HalfEdgeUtils::printMesh(mesh);
		//	std::cout << std::endl;
		currentInd++;
	}
	else
		std::cout << "Couldnt generate another tetrahedron" << std::endl;

	return nullptr;
}
bool TetrahedralizationContext::addNextTetra(bool checkifUsed) {
	Mesh* totalMesh = volume.totalMesh;

	vector<Geometry::Vertex*> &vertices = totalMesh->vertices;

	Geometry::Vertex* closest = vertices[0];
	Facet* facet = openFacets[0];
	int facetIndex = 0;
	float shortestDistance = -1;
	Mesh* m = nullptr;
	float bestVol = -1;
#pragma omp paralllel for
	for (int i = 0; i < openFacets.size();i++) {

		Geometry::Facet* testFacet = openFacets[i];
		vector<int> &facetPartition = partitions[testFacet->externalIndex];

		for (int j = 0; j < facetPartition.size();j++) {
			int vertexIndex = facetPartition[j];
			Geometry::Vertex* vertex = totalMesh->vertices[vertexIndex];
			if (!usedVertices[vertex->externalIndex] || !checkifUsed) {
				float vol = HalfEdgeUtils::getFacetPointVolume(testFacet, vertex, positions);
				
				if (HalfEdgeUtils::facetPointsTo(*testFacet, *vertex, positions) && vol > MIN_VOLUME) {

					float distance = HalfEdgeUtils::distanceToFacet(positions, *vertex, *testFacet);
					if (distance < shortestDistance || shortestDistance == -1) {
						
	#pragma omp critical
						{
							shortestDistance = distance;
							closest = vertex;
							facet = testFacet;
							facetIndex = i;
							bestVol = vol;
						}
					}

				}
			}
		}

	}

	if (shortestDistance == -1) {
	//	std::cout << "Could not add another face"<<std::endl;
		return true;
	}
	else {
		
		Facet* twinFacet = HalfEdgeUtils::constructTwinFacet(facet);
		Mesh* m = HalfEdgeUtils::constructTetrahedron(*closest, *twinFacet,totalMesh->vertices, positions);

		for (int i = 0; i < m->vertices.size();i++) {
			usedVertices[m->vertices[i]->externalIndex] = true;
		}
		HalfEdgeUtils::addMeshToVolume(m, &volume);
	//	std::cout << "Adding mesh" << m->internalIndex << " from facet " << facet->externalIndex << std::endl;
	//	HalfEdgeUtils::printMesh(m);
	//	std::cout << std::endl;


		for (int i = 0; i < m->facets.size();i++) {
			Facet* f = m->facets[i];
			if (f != twinFacet) {
				openFacets.push_back(f);
			}
			partitions.push_back(HalfEdgeUtils::makeFacetPartition(f, positions, partitions[facet->externalIndex]));
		}
	
		if (m != nullptr && !checkifUsed) {

			vector<Facet*>::iterator ite = remove_if(openFacets.begin(), openFacets.end(), [&](Facet* f)->bool {
				return (f->twin != nullptr);
			});
			openFacets.erase(ite, openFacets.end());
			
			return false;
		}
		else return false;

	}
}

vector<Geometry::Mesh*> TetrahedralizationContext::fillUpGaps(Geometry::Mesh* mesh) {
	return vector<Geometry::Mesh*>(0);
	
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
						

						HalfEdgeUtils::addMeshToVolume(m, &volume);
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