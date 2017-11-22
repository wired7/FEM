#include "HalfSimplices.h"
#include <iostream>
#include <stack>
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
	
	cout << "NUM VERTICES " << vertices.size() << endl;

	vector<vector<HalfEdge*>> temp(vertices.size());

	for (int i = 0; i < indices.size(); i += verticesPerFacet)
	{

		Vertex* v1 = vertexLookup(indices[i]);
		Vertex* v2 = vertexLookup(indices[i + 1]);
		Vertex* v3 = vertexLookup(indices[i + 2]);

		auto hE1 = new HalfEdge(v1, v2, indices[i], indices[i+1]);
		auto hE2 = new HalfEdge(v2, v3, indices[i+1], indices[i+2]);
		auto hE3 = new HalfEdge(v3, v1, indices[i+2], indices[i]);

		hE1->internalIndex = halfEdges.size();
		hE2->internalIndex = halfEdges.size() + 1;
		hE3->internalIndex = halfEdges.size() + 2;
					 										 
		hE1->next = hE2;						 
		hE2->next = hE3;						 
		hE3->next = hE1;						 

		hE1->previous = hE3;
		hE2->previous = hE1;
		hE3->previous=  hE2;

		auto facet = new Facet();
		facet->halfEdge = hE1;

		temp[hE1->start].push_back(hE1);
		temp[hE2->start].push_back(hE2);
		temp[hE3->start].push_back(hE3);

		halfEdges.push_back(hE1);
		halfEdges.push_back(hE2);
		halfEdges.push_back(hE3);

		facets.push_back(facet);
	}

	cout << "NUM EDGES " << halfEdges.size() / 2 << endl;
	system("PAUSE");

	for (int i = 0; i < halfEdges.size(); i++) {
		HalfEdge* he = halfEdges[i];
		if (he->twin == nullptr) {
			vector<HalfEdge*> &list = temp[he->end];
			for (int j = 0; j < list.size(); j++) {
				HalfEdge * heTemp = list[j];

				cout << heTemp->start << " " << heTemp->end << endl;
				cout << he->end << " " << he->start << endl;
				if (heTemp->end == he->start) {
					heTemp->twin = he;
					he->twin = heTemp; 
					cout << "FOUND!" << endl<<endl;
					
					break;
				}

			}
			cout  << endl << endl << endl;

		}
	}
	int numNoTwin = 0;
	for (int i = 0; i < halfEdges.size(); i++) {
		if (halfEdges[i]->twin == nullptr)
			numNoTwin++;
	}
	std::cout << "Num no twin " << numNoTwin << std::endl;
	/*
	vector<HalfEdge*> lonelyEdges;
	vector<vector<HalfEdge*>> temp2(vertices.size());

	for (int i = 0; i < halfEdges.size(); i++)
	{
		if (halfEdges[i]->twin == nullptr)
		{
			auto halfEdge = new HalfEdge(halfEdges[i]->vertex, halfEdges[i]->previous->vertex,-1,-1);
			halfEdge->internalIndex = halfEdges.size() + lonelyEdges.size();
			halfEdges[i]->twin = halfEdge;
			halfEdge->twin = halfEdges[i];
			lonelyEdges.push_back(halfEdge);
			temp2[halfEdge->start].push_back(halfEdge);
		}
	}
	std::cout << "Lonely edges " << lonelyEdges.size()<<std::endl;

	if (lonelyEdges.size() > 0) {
		int numHoles = 0;
		vector<HalfEdge*> hole;
		vector<HalfEdge*> split;
		vector<HalfEdge*> toBeRemoved;
		int offset = halfEdges.size();
		while (lonelyEdges.size() > 0) {

			vector<bool> visited(lonelyEdges.size(), false);
			hole.push_back(lonelyEdges[0]);
			split.push_back(lonelyEdges[0]);
			do {
				int end = hole[hole.size() - 1]->end;
				vector<HalfEdge*> & edges = temp2[end];
				int numNotVisited = 0;
				HalfEdge* toPush = nullptr;
				for (int i = 0; i < edges.size(); i++) {
					HalfEdge* potentialNext = edges[i];
					if (!visited[potentialNext->internalIndex - offset]) {
						numNotVisited++;
						toPush = potentialNext;
					}
				}
				if (split[split.size() - 1] == toPush) {
					std::cout << "chinchilla++";
					HalfEdge* lastFace = hole[hole.size() - 1];
					lastFace->next = toPush;
					toPush->previous = lastFace;
					for (int i = hole.size() - 1; i > 1; i++) {
						if (hole[i] == toPush) {
							visited[toPush->internalIndex - offset] = true;
							break;
						}

						hole[i]->previous = hole[i - 1];
						hole[i - 1]->next = hole[i];
						visited[hole[i]->internalIndex - offset] = true;
						toBeRemoved.push_back(hole[i]);
						hole.pop_back();

					}
					split.pop_back();

				}
				else {

					hole.push_back(toPush);
					if (numNotVisited > 1) {
						split.push_back(toPush);
					}
				}

			} while (split.size() > 0);

			for (int i = 0; i < toBeRemoved.size(); i++) {

				for (int j = 0; j < lonelyEdges.size(); j++) {
					if (toBeRemoved[i] == lonelyEdges[j]) {
						lonelyEdges.erase(lonelyEdges.begin() + j);
						break;
					}

				}
			}
		}
	}

	*/

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
	int i = 0;
	for (; i < vertices.size(); i++)
	{
		if (vertices[i]->externalIndex > vertex->externalIndex)
		{
			break;
		}
	}

	vertices.insert(vertices.begin() + i, vertex);
}