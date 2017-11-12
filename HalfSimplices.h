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
		HalfEdge* halfEdge;
		int externalIndex;
		Vertex(int externalIndex) : externalIndex(externalIndex) {};
	};

	struct Edge
	{
	public:
		HalfEdge* halfEdge;
	};

	struct Facet
	{
	public:
		HalfEdge* halfEdge;
	};

	struct HalfEdge
	{
	public:
		HalfEdge* twin;
		HalfEdge* next;
		Vertex* vertex;
		Edge* edge;
		Facet* facet;
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
		vector<Edge*> edges;
		vector<Facet*> facets;
		HalfSimplices(vector<GLuint> indices, int verticesPerFacet);
		~HalfSimplices();
		Vertex* vertexLookup(int externalIndex);
		void vertexInsert(Vertex* vertex);
	};

}