#include "HalfSimplices.h"
#include <iostream>

using namespace std;
using namespace HalfEdge;

HalfSimplices::HalfSimplices(vector<GLuint> indices, int verticesPerFacet)
{
	for (int i = 0; i < indices.size(); i++)
	{
		if (161 == indices[i])
			int x = 0;
		Vertex* vertex = vertexLookup(indices[i]);

		if (vertex == nullptr)
		{
			vertex = new Vertex(indices[i]);
			vertexInsert(vertex);
		}
		if ((vertex != vertexLookup(vertex->externalIndex))) {
			std::cout << i << std::endl;
		}
	}

	std::cout << "Fake Num of Verts: " << vertices.size() << std::endl;

	vector<vector<HalfEdge*>> temp(vertices.size());

	for (int i = 0; i < indices.size(); i += 3)
	{

		Vertex* v1 = vertexLookup(indices[i]);
		Vertex* v2 = vertexLookup(indices[i + 1]);
		Vertex* v3 = vertexLookup(indices[i + 2]);

		auto hE1 = new HalfEdge(v1, v2);
		auto hE2 = new HalfEdge(v2, v3);
		auto hE3 = new HalfEdge(v3, v1);
					 										 
		hE1->next = hE2;						 
		hE2->next = hE3;						 
		hE3->next = hE1;						 

		hE1->previous = hE3;
		hE2->previous = hE1;
		hE3->previous= hE2;

		auto facet = new Facet();
		facet->halfEdge = hE1;

		temp[v1->externalIndex].push_back(hE1);
		temp[v2->externalIndex].push_back(hE2);
		temp[v3->externalIndex].push_back(hE3);

		halfEdges.push_back(hE1);
		halfEdges.push_back(hE2);
		halfEdges.push_back(hE3);

		facets.push_back(facet);
	}
	for (int i = 0; i < halfEdges.size(); i++) {
		HalfEdge* he = halfEdges[i];
		vector<HalfEdge*> &list = temp[he->end];

		for (int j = 0; j < list.size(); j++) {
			HalfEdge * heTemp = list[j];

			if (heTemp->end == he->start) {
				heTemp->twin = he;
				he->twin = heTemp;
			}
		}
	}
}


HalfSimplices::~HalfSimplices()
{

}


int HalfSimplices::binarySearch(int externalIndex, vector<Vertex*>& data, int min, int max)
{
	if (min > max)
	{
		return -1;
	}

	int half = (max + min) / 2;

	if (data[half]->externalIndex == externalIndex)
	{
		return half;
	}

	if (max - min > 0)
	{
		if (data[half]->externalIndex > externalIndex)
		{
			return binarySearch(externalIndex, data, min, half - 1);
		}
		else
		{
			return binarySearch(externalIndex, data, half + 1, max);
		}
	}
	else
	{
		return -1;
	}
}

void HalfSimplices::binaryInsert(Vertex* vertex, vector<Vertex*>& data, int min, int max)
{
	if (min > max)
	{
		return;
	}

	int half = (max + min) / 2;

	if (data[half]->externalIndex == vertex->externalIndex)
	{
		return;
	}

	if (max - min == 0)
	{
		vertices.insert(vertices.begin() + half, vertex);
		return;
	}

	if (data[half]->externalIndex > vertex->externalIndex)
	{
		binaryInsert(vertex, data, min, half - 1);
	}
	else
	{
		binaryInsert(vertex, data, half + 1, max);
	}
}

Vertex* HalfSimplices::vertexLookup(int externalIndex)
{
	if (vertices.size() == 0)
	{
		return nullptr;
	}

	int internalIndex = binarySearch(externalIndex, vertices, 0, vertices.size() - 1);

	if (internalIndex > -1)
	{
		return vertices[internalIndex];
	}
	else
	{
		return nullptr;
	} 
}

void HalfSimplices::vertexInsert(Vertex* vertex)
{
	if (vertices.size() > 0)
	{
		binaryInsert(vertex, vertices, 0, vertices.size() - 1);
	}
	else
	{
		vertices.push_back(vertex);
	}
}