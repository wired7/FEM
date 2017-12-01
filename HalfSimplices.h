#pragma once
#include <vector>
#include <glew.h>
#include <glm.hpp>

using namespace std;
using namespace glm;
#define dDEBUG_HALFSIMPLICES 
namespace Geometry {

	struct Volume;
	struct HalfEdge;
	struct Facet;

	struct Vertex
	{
	public:
		// Arbitrary halfedge that comes to it
		HalfEdge* halfEdge;
		int externalIndex;
		Vertex(int externalIndex) : externalIndex(externalIndex) {};
	};


	typedef Facet Hole;

	struct HalfEdge
	{
	public:
		int internalIndex;
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
		Volume* volume;
		Facet* twin;
		Facet* next;
		Facet* previous;
		int internalIndex;

		vector<int> getIndices() {
			vector<int> indices(0);
			HalfEdge* h = halfEdge;
			HalfEdge * n = h;
			while (n->next != h) {
				indices.push_back(n->start);
				n = n->next;
			}
			return indices;
		}
	};
	struct Volume
	{
	public:
		Facet* Facet;
	};

	class HalfSimplices
	{
	private:
		int binarySearch(int externalIndex, vector<Vertex*>& data, int min, int max);
	public:
		vector<Vertex*> vertices;
		vector<HalfEdge*> halfEdges;
		vector<Facet*> facets;
		vector<Hole*> holes;
		vector<Volume*> tetras;
		vector<Volume*> tetraHoles;

		HalfSimplices(vector<GLuint> indices, int verticesPerFacet);
		~HalfSimplices();
		Vertex* vertexLookup(int externalIndex);
		void vertexInsert(Vertex* vertex);
	};

}