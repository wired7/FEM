#include "ReferencedGraphicsObject.h"

using namespace Graphics;
ReferenceManager::ReferenceManager()
{

}

ReferenceManager::~ReferenceManager()
{

}

int ReferenceManager::assignNewGUID(DecoratedGraphicsObject* gObject, int indexWithinObject)
{
	for (int i = 0; i < managedGraphicsObject.size(); i++)
	{
		if (managedGraphicsObject[i].first == gObject)
		{
			for (int j = 0; j < managedGraphicsObject[i].second.size(); j++)
			{
				if (managedGraphicsObject[i].second[j].first == indexWithinObject)
				{
					return managedGraphicsObject[i].second[j].second;
				}
			}

			managedGraphicsObject[i].second.push_back(pair<int, int>(indexWithinObject, count));
			return count++;
		}
	}

	managedGraphicsObject.push_back(pair<DecoratedGraphicsObject*, vector<pair<int, int>>>(gObject, { pair<int, int>(indexWithinObject, count) }));
	return count++;
}

int ReferenceManager::assignNewGUID(void)
{
	return count++;
}