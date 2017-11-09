#pragma once
#include "GraphicsObject.h"
#include <utility>

using namespace std;

//Stores and manages reference graphics object and instance index within it associated with map index
class ReferenceManager
{
private:
	int count = 0;
	vector<pair<DecoratedGraphicsObject*, int>> managedGraphicsObject;
public:
	vector<pair<int, int>> entityMap;
	int assignNewGUID(DecoratedGraphicsObject* gObject, int indexWithinObject);
	pair<DecoratedGraphicsObject*, int> getInstance(unsigned int guid);
	void deleteRange(DecoratedGraphicsObject*, int minIndex, int maxIndex);
};

template<class T, class S> class ReferencedGraphicsObject : InstancedMeshObject<T, S>
{
public:
	ReferencedGraphicsObject();
	~ReferencedGraphicsObject();

};

template<class T, class S> ReferencedGraphicsObject<T, S>::ReferencedGraphicsObject()
{
}


template<class T, class S> ReferencedGraphicsObject<T, S>::~ReferencedGraphicsObject()
{
}

class SelectionManager
{
public:
	vector<pair<DecoratedGraphicsObject*, int>> selectedGraphicsObject;
};