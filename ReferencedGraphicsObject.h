#pragma once
#include "GraphicsObject.h"
#include <utility>

using namespace std;

//Stores and manages reference graphics object and instance index within it associated with map index
class ReferenceManager
{
private:
	unsigned int count = 0;
	vector<pair<DecoratedGraphicsObject*, vector<pair<int, int>>>> managedGraphicsObject;
public:
	ReferenceManager();
	~ReferenceManager();
	int assignNewGUID(DecoratedGraphicsObject* gObject, int indexWithinObject = 0);
	pair<DecoratedGraphicsObject*, int> getInstance(unsigned int guid);
	void deleteRange(DecoratedGraphicsObject*, int minIndex, int maxIndex);
};

template<class T, class S> class ReferencedGraphicsObject : public InstancedMeshObject<T, S>
{
public:
	ReferencedGraphicsObject(ReferenceManager* refMan, DecoratedGraphicsObject* child, int numInstances, string bufferSignature, int divisor);
	~ReferencedGraphicsObject();
};

template<class T, class S>
ReferencedGraphicsObject<T, S>::ReferencedGraphicsObject(ReferenceManager* refMan, DecoratedGraphicsObject* child, int numInstances, string bufferSignature, int divisor) :
	InstancedMeshObject<T, S>(child, bufferSignature, divisor)
{
	for (int i = 0; i < numInstances; i++)
	{
		extendedData.push_back(refMan->assignNewGUID(this, i));
	}

	glDeleteBuffers(1, &VBO);
	bindBuffers();
}


template<class T, class S> ReferencedGraphicsObject<T, S>::~ReferencedGraphicsObject()
{
}

class SelectionManager
{
public:
	vector<pair<DecoratedGraphicsObject*, int>> selectedGraphicsObject;
	ReferenceManager* refMan;
};