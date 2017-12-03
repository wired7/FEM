#pragma once
#include <vector>
#include <glew.h>
#include <glm.hpp>

using namespace std;
using namespace glm;
#define dDEBUG_HALFSIMPLICES 
namespace Geometry {

	struct HalfEdge;
	struct Facet;
	class Mesh;
	class VolumetricMesh;
	struct Vertex
	{
	public:
		// Arbitrary halfedge that comes to it
		HalfEdge* halfEdge;
		int externalIndex;
		Vertex(int externalIndex) : externalIndex(externalIndex) {};
		Vertex() {};
	};


	typedef Facet Hole;

	struct HalfEdge
	{
	public:
		int internalIndex;
		int externalIndex;
		int start, end;
		HalfEdge* twin;
		HalfEdge* next;
		HalfEdge* previous;
		// this is the vertex it points to
		Vertex* vertex;
		Facet* facet;
		HalfEdge(Vertex* v1, Vertex* v2) : twin(nullptr), next(nullptr), previous(nullptr){
			vertex = v2;
			v2->halfEdge = this;
			start = v1->externalIndex;
			end = v2->externalIndex;
		};

	};


	struct Facet
	{
	public:
		HalfEdge* halfEdge;
		Mesh* mesh;
		Facet* twin;
		Facet* next;
		Facet* previous;
		int internalIndex;
		int externalIndex;
		Facet() {};
		Facet(HalfEdge * he) : twin(nullptr), next(nullptr), previous(nullptr), mesh(nullptr), internalIndex(-1), externalIndex(-1)
		{
			halfEdge = he;
			HalfEdge* nextEdge = he;
			int i = 0;
			do {
				nextEdge->internalIndex = i++;
				nextEdge = nextEdge->next;
			} while (nextEdge != he);

		}
	};

	class Mesh
	{
	private:
		int binarySearch(int externalIndex, vector<Vertex*>& data, int min, int max);
	public:
		VolumetricMesh * volume;
		vector<Vertex*> vertices;
		vector<HalfEdge*> halfEdges;
		vector<Facet*> facets;
		vector<Hole*> holes;
		int internalIndex;
		Mesh(vector<GLuint> indices, int verticesPerFacet);
		Mesh() {};
		~Mesh();
		Vertex* vertexLookup(int externalIndex);
		void vertexInsert(Vertex* vertex);
		bool isOutsideOrientated;

	};

	struct VolumetricMesh {
	public:
		vector<Mesh*> meshes;
		void addMesh(Mesh * mesh) {
			mesh->internalIndex = meshes.size();
			meshes.push_back(mesh);
			mesh->volume = this;


			for (int i = 0; i < mesh->facets.size();i++) {
				mesh->facets[i]->externalIndex = totalMesh->facets.size();
				totalMesh->facets.push_back(mesh->facets[i]);
			}
			for (int i = 0; i < mesh->halfEdges.size();i++) {
				mesh->halfEdges[i]->externalIndex = totalMesh->halfEdges.size();
				totalMesh->halfEdges.push_back(mesh->halfEdges[i]);
			}
		}

		Mesh* totalMesh;
		vector<glm::vec3> positions;
	};

}