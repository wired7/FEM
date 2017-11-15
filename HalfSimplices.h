#pragma once
#include <vector>
#include <glew.h>
#include <glm.hpp>

using namespace std;
using namespace glm;

namespace HalfEdge {

	struct HalfEdge;

	struct Vertex
	{
	public:
		// Arbitrary halfedge that comes to it
		HalfEdge* halfEdge;
		int externalIndex;
		Vertex(int externalIndex) : externalIndex(externalIndex) {};
	};

	struct Facet
	{
	public:
		HalfEdge* halfEdge;
		int internalIndex;

	private:
		int internalIndexCounter = 0;
	};


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
		HalfEdge(Vertex* v1, Vertex* v2) {
			vertex = v2;
			v2->halfEdge = this;
			start = v1->externalIndex;
			end = v2->externalIndex;
		};
	};

	struct HalfFacet;

	struct Tetra
	{
	public:
		HalfFacet* halfFacet;
	};

	struct HalfFacet : public HalfEdge
	{
	public:
		Tetra* tetra;
	};

	class HalfSimplices
	{
	private:
		int binarySearch(int externalIndex, vector<Vertex*>& data, int min, int max);
		void binaryInsert(Vertex* vertex, vector<Vertex*>& data, int min, int max);
	public:
		vector<Vertex*> vertices;
		vector<HalfEdge*> halfEdges;
		vector<Facet*> facets;
		vector<Tetra*> tetras;
		HalfSimplices(vector<GLuint> indices, int verticesPerFacet);
		~HalfSimplices();
		Vertex* vertexLookup(int externalIndex);
		void vertexInsert(Vertex* vertex);
	};

}