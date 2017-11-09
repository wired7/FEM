#pragma once
#include <vector>
#include <string>

using namespace std;

template <class T> class Decorator
{
public:
	T* child;
	string signature;

	Decorator();
	Decorator(T* child, string signature);
	~Decorator();

	virtual T* make() = 0;
	virtual T* clone();
	// TODO: Recursive Lookup of signature through children
	virtual T* signatureLookup(string signature);
	virtual vector<T*>* signatureLookup(vector<string>& signatures);
	// TODO: Look for buffer with signature and call the function at occurrence
	virtual bool signatureCallback(string signature/*, function callback*/);
	virtual bool signatureCallback(vector<string> signatures/*, function callback*/);
	// TODO: Look for the signature recursively, add it on top of signature owner, reorganize layout indices and bind all shifted buffers, including the new one
	virtual T* addOnTopOf(string signature, T* gObject);
	// TODO: Look for the signature recursively, unbind it from the VAO, delete VBO, return its child, and reorganize layout indices from any child and parent
	// so that they're sequential, and rebind every buffer that was shifted to the proper layout number
	virtual T* remove(string signature);
	virtual T* remove(vector<string> signatures);

	virtual string printOwnProperties() = 0;
	friend ostream& operator<< (ostream& stream, const T* gObject);
};

template <class T> Decorator<T>::Decorator()
{
}

template <class T> Decorator<T>::Decorator(T* child, string signature) : child(child), signature(signature)
{
}

template <class T> Decorator<T>::~Decorator()
{
}

template <class T> T* Decorator<T>::clone()
{
	T* copy = make();

	if (child != nullptr)
	{
		copy->child = child->clone();
	}

	return copy;
}

template <class T> T* Decorator<T>::signatureLookup(string signature)
{
	vector<string> signatures;
	signatures.push_back(signature);

	vector<T*>* v = signatureLookup(signatures);

	if (v->size())
	{
		return v->at(0);
	}
	else
	{
		return nullptr;
	}
}

template <class T> vector<T*>* Decorator<T>::signatureLookup(vector<string>& signatures)
{
	vector<T*>* v;

	if (child == nullptr)
	{
		v = new vector<T*>();
	}
	else
	{
		v = child->signatureLookup(signatures);
	}

	for (int i = 0; i < signatures.size(); i++)
	{
		if (signatures[i] == signature)
		{
			signatures.erase(signatures.begin() + i);
			v->push_back((T*)this);
			break;
		}
	}

	return v;
}

template <class T> bool Decorator<T>::signatureCallback(string signature/*, function callback*/)
{
	return false;
}

template <class T> bool Decorator<T>::signatureCallback(vector<string> signatures/*, function callback*/)
{
	return false;
}

template <class T> T* Decorator<T>::addOnTopOf(string signature, T* gObject)
{
	return NULL;
}

template <class T> T* Decorator<T>::remove(string signature)
{
	return NULL;
}

template <class T> T* Decorator<T>::remove(vector<string> signatures)
{
	return NULL;
}

template <class T> std::ostream& operator <<(std::ostream& stream, const T* gObject)
{
	string outString = "";

	int indentCount = 0;

	do
	{
		for (int i = 0; i < indentCount; i++)
		{
			outString += "\t";
		}

		outString += gObject->signature + "\n";

		for (int i = 0; i < indentCount; i++)
		{
			outString += "\t";
		}

		outString += "{\n";

		indentCount++;

		for (int i = 0; i < indentCount; i++)
		{
			outString += "\t";
		}

		outString += ((T*)gObject)->printOwnProperties();

		gObject = (T*)(gObject->child);
	} while (gObject != nullptr);

	for (int i = indentCount - 1; i >= 0; i--)
	{
		for (int j = 0; j < i; j++)
		{
			outString += "\t";
		}

		outString += "}\n";
	}

	stream << outString;

	return stream;
}