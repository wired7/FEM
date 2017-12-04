#pragma once
#include "HalfSimplices.h"
#include <vector>

using namespace std;

class VertexNode
{
public:
	Geometry::Vertex* vertex;
	vector<VertexNode*> neighbors;
	VertexNode(Geometry::Vertex* vertex);
	~VertexNode();
};

class MeshGraph
{
public:
	vector<VertexNode*> nodes;
	MeshGraph() {};
	~MeshGraph() {};
};

