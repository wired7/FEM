#pragma once

#include "GraphicsObject.h"
#include "TextureManager.h"
#include <iostream>

using namespace Assimp;
using namespace Graphics;

DecoratedGraphicsObject::DecoratedGraphicsObject(Graphics::DecoratedGraphicsObject* child, string bufferSignature) : Decorator(child, bufferSignature)
{

}

DecoratedGraphicsObject::~DecoratedGraphicsObject()
{

}

void DecoratedGraphicsObject::updateBuffers(vector<string> bufferSignatures)
{

}

void DecoratedGraphicsObject::updateBuffersPartially(int minBufferIndex, int maxBufferIndex)
{

}

void DecoratedGraphicsObject::updateBuffersPartially(int minBufferIndex, int maxBufferIndex, vector<string> bufferSignatures)
{

}

void DecoratedGraphicsObject::enableBuffers(void)
{
	glBindVertexArray(VAO);

//	updateIfDirty();

	int index = 0;
	if (child != nullptr)
	{
		index = child->layoutCount;
		child->enableBuffers();
	}

	for (int i = index; i < layoutCount; i++)
	{
		glEnableVertexAttribArray(i);
	}
}

string DecoratedGraphicsObject::printOwnProperties(void)
{
	return to_string(layoutCount) + "\n";
}

mat4 DecoratedGraphicsObject::getModelMatrix(void)
{
	return model;
}

MeshObject::MeshObject() : DecoratedGraphicsObject(nullptr, "VERTEX")
{
	layoutCount = 2;
}


MeshObject::MeshObject(vector<Vertex> vertices, vector<GLuint> indices) : DecoratedGraphicsObject(nullptr, "VERTEX"), vertices(vertices), indices(indices)
{
	layoutCount = 2;
	bindBuffers();
}

MeshObject::~MeshObject(void)
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}

void MeshObject::commitVBOToGPU()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &(vertices[0]), GL_DYNAMIC_DRAW);

	if (indices.size())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &(indices[0]), GL_DYNAMIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	glBindVertexArray(0);

	commitedVertexCount = vertices.size();
	commitedIndexCount = indices.size();
}

void MeshObject::bindBuffers(void)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	commitVBOToGPU();
}

void MeshObject::updateBuffers(void)
{
	glBindVertexArray(VAO);
	commitVBOToGPU();
}

void MeshObject::addVertex(vec3 pos, vec3 normal)
{
	vertices.push_back(Vertex(pos, normal));
	dirty = true;
}

void MeshObject::draw(void)
{
	glDrawElements(GL_TRIANGLES, commitedIndexCount, GL_UNSIGNED_INT, 0);
 	glBindVertexArray(0);
}

void MeshObject::updateIfDirty(void)
{
	if (dirty)
	{
		if (vertices.size() != commitedVertexCount || indices.size() != commitedIndexCount)
		{
/*			glBindVertexArray(VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);

			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			commitVBOToGPU();*/
		}
		else
		{
			updateBuffers();
		}

		dirty = false;
	}
}

ImportedMeshObject::ImportedMeshObject(const char* string) : MeshObject()
{
	loadFile(string);

	vec3 avg(0.0f, 0.0f, 0.0f);
	vec3 min(INFINITY, INFINITY, INFINITY);
	vec3 max(-INFINITY, -INFINITY, -INFINITY);
	for (int i = 0; i < vertices.size(); i++)
	{
		avg += vertices[i].position;

		for (int j = 0; j < 3; j++)
		{
			if (vertices[i].position[j] > max[j])
			{
				max[j] = vertices[i].position[j];
			}
			else if (vertices[i].position[j] < min[j])
			{
				min[j] = vertices[i].position[j];
			}
		}
	}

	vec3 diff = max - min;

	if (vertices.size())
	{
		avg /= vertices.size();
	}

	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].position -= avg;
		vertices[i].position /= (20.0f * diff.length());
	}

	bindBuffers();
}

void ImportedMeshObject::loadFile(const char* filePath)
{
	Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!scene)
	{
		return;
	}

	for (int i = 0, count = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			vec3 pos(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vec3 normal(0, 0, 1);
			if (mesh->HasNormals())
			{
				normal = vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			}

			addVertex(pos, normalize(normal));
		}

		for (int j = 0; j < mesh->mNumFaces; j++)
		{
			for (int k = 0; k < mesh->mFaces[j].mNumIndices; k++)
			{
				indices.push_back(count + mesh->mFaces[j].mIndices[k]);
			}
		}

		count += mesh->mNumVertices;
	}
}

TexturedMeshObject::TexturedMeshObject(DecoratedGraphicsObject* child, char* filename, vector<vec2> data) : ExtendedMeshObject(child, data, "UVTEXTURE")
{
	loadTexture(filename);
}

TexturedMeshObject::TexturedMeshObject(DecoratedGraphicsObject* child, GLuint texture, vector<vec2> data) : ExtendedMeshObject(child, data, "UVTEXTURE"), texture(texture)
{

}

void TexturedMeshObject::loadTexture(char* filePath)
{
	std::ifstream in(filePath, std::ios::in);
	assert(in.is_open());

	texture = TextureManager::instance()->addTexture(filePath);
}

void TexturedMeshObject::enableBuffers(void)
{
	DecoratedGraphicsObject::enableBuffers();
	glBindTexture(GL_TEXTURE_2D, texture);
}