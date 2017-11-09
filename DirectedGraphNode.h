#pragma once
#include <vector>

using namespace std;

template<class T> class DirectedGraphNode
{
protected:
	vector<T*> neighbors;
	vector<T*> parents;
public:
	string signature;
	DirectedGraphNode();
	DirectedGraphNode(vector<T*> neighbors, string signature);
	DirectedGraphNode(string signature);
	~DirectedGraphNode();
	void addNeighbor(T* neighbor);
	void addNeighbors(vector<T*> neighbors);
	vector<T*> getLeaves();
	virtual T* clone();
	// TODO: Recursive Lookup of signature through children
	virtual T* signatureLookup(string signature);
	virtual vector<T*>* signatureLookup(vector<string>& signatures);
	// TODO: Look for buffer with signature and call the function at occurrence
	virtual bool signatureCallback(string signature/*, function callback*/);
	virtual bool signatureCallback(vector<string> signatures/*, function callback*/);
	// TODO: Look for the signature recursively, unbind it from the VAO, delete VBO, return its child, and reorganize layout indices from any child and parent
	// so that they're sequential, and rebind every buffer that was shifted to the proper layout number
	virtual T* remove(string signature);
	virtual T* remove(vector<string> signatures);
};

template<class T> DirectedGraphNode<T>::DirectedGraphNode()
{
}

template<class T> DirectedGraphNode<T>::DirectedGraphNode(vector<T*> neighbors, string signature) : neighbors(neighbors), signature(signature)
{
}

template<class T> DirectedGraphNode<T>::DirectedGraphNode(string signature) : signature(signature)
{
}

template<class T> DirectedGraphNode<T>::~DirectedGraphNode()
{

}

template<class T> void DirectedGraphNode<T>::addNeighbor(T* neighbor)
{
	neighbors.push_back(neighbor);
	neighbor->parents.push_back((T*)this);
}

template <class T> T* DirectedGraphNode<T>::clone()
{
	return nullptr;
}

template <class T> T* DirectedGraphNode<T>::signatureLookup(string signature)
{
	if (signature == this->signature)
	{
		return (T*)this;
	}

	for (int i = 0; i < neighbors.size(); i++)
	{
		auto output = neighbors[i]->signatureLookup(signature);

		if (output != nullptr)
		{
			return output;
		}
	}

	return nullptr;
}

template <class T> vector<T*>* DirectedGraphNode<T>::signatureLookup(vector<string>& signatures)
{
	return nullptr;
}

template <class T> bool DirectedGraphNode<T>::signatureCallback(string signature/*, function callback*/)
{
	return false;
}

template <class T> bool DirectedGraphNode<T>::signatureCallback(vector<string> signatures/*, function callback*/)
{
	return false;
}


template <class T> T* DirectedGraphNode<T>::remove(string signature)
{
	return NULL;
}

template <class T> T* DirectedGraphNode<T>::remove(vector<string> signatures)
{
	return NULL;
}