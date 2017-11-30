#pragma once
#include "HalfSimplices.h"

static class HalfEdgeUtils
{
public:
	static vector<pair<int, int>> getEdges(HalfEdge::HalfSimplices* hS);
};

