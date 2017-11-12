#include "HalfSimplices.h"
#include <iostream>

using namespace std;
using namespace HalfEdge;

HalfSimplices::HalfSimplices(vector<GLuint> indices, int verticesPerFacet)
{
	for (int i = 0; i < indices.size(); i++)
	{
		Vertex* vertex = vertexLookup(indices[i]);

		if (vertex == nullptr)
		{
			vertex = new Vertex(indices[i]);
			vertexInsert(vertex);
		}
	}

	for(int )
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