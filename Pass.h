#pragma once
#include <vector>
#include <glew.h>
#include <glfw3.h>
#include <utility>
#include "GeometricalMeshObjects.h"
#include "DirectedGraphNode.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Context.h"

using namespace std;


class Pass : public DirectedGraphNode<Pass>
{
protected:
	int incomingCount = 0;
	int currentCount = 0;
	vector<ShaderProgramPipeline*> shaderPipelines;
	virtual void executeOwnBehaviour() = 0;
public:
	vector<vector<tuple<string, GLuint, GLuint, UniformType>>> uniformIDs;
	vector<vector<tuple<int, GLfloat*>>> floatTypeUniformPointers;
	vector<vector<tuple<int, GLuint*>>> uintTypeUniformPointers;
	vector<vector<tuple<int, GLint*>>> intTypeUniformPointers;
	vector<vector<tuple<int, GLuint>>> uintTypeUniformValues;
	Pass();
	Pass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature);
	Pass(vector<ShaderProgramPipeline*> shaderPipelines, string signature);
	~Pass();
	virtual void execute(void);
	virtual void registerUniforms(void);
	virtual int getUniformIndexBySignature(int programIndex, string signature);
	template<typename T> void updatePointerBySignature(int programIndex, string signature, T* pointer);
	template<typename T> void updateValueBySignature(int programIndex, string signature, T value);
	virtual void addNeighbor(Pass* neighbor);
};

// IMPLEMENT FOR OTHER TYPES!!!
template<typename T> void Pass::updatePointerBySignature(int programIndex, string signature, T* pointer)
{
	if (is_same<T, GLfloat>::value || is_same<T, float>::value)
	{
		for (int i = 0; i < floatTypeUniformPointers[programIndex].size(); i++)
		{
			if (get<0>(uniformIDs[programIndex][get<0>(floatTypeUniformPointers[programIndex][i])]) == signature)
			{
				get<1>(floatTypeUniformPointers[programIndex][i]) = pointer;
				return;
			}
		}

		floatTypeUniformPointers[programIndex].push_back(tuple<int, T*>(getUniformIndexBySignature(programIndex, signature), pointer));
	}
}

template<typename T> void Pass::updateValueBySignature(int programIndex, string signature, T value)
{
	if (is_same<T, GLuint>::value || is_same<T, unsigned int>::value)
	{
		for (int i = 0; i < uintTypeUniformValues[programIndex].size(); i++)
		{
			if (get<0>(uniformIDs[programIndex][get<0>(uintTypeUniformValues[programIndex][i])]) == signature)
			{
				get<1>(uintTypeUniformValues[programIndex][i]) = value;
				return;
			}
		}

		uintTypeUniformValues[programIndex].push_back(tuple<int, T>(getUniformIndexBySignature(programIndex, signature), value));
	}
}

class RenderPass : public Pass
{
protected:
	vector<vector<Graphics::DecoratedGraphicsObject*>> renderableObjects;
	bool terminal;
	virtual void initFrameBuffers(void) = 0;
	virtual void clearBuffers(void);
	virtual void configureGL(void);
	virtual void renderObjects(int programIndex);
	virtual void setupObjectwiseUniforms(int programIndex, int index);
	virtual void executeOwnBehaviour(void);
public:
	DecoratedFrameBuffer* frameBuffer;
	RenderPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	RenderPass(vector<ShaderProgramPipeline*> shaderPipelines, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	~RenderPass();
	// TODO: Give the user the ability to apply renderable objects to any inner pass for rendering before the call
	virtual void clearRenderableObjects(int programIndex);
	virtual void clearRenderableObjects(int programIndex, string signature) {};
	virtual void setRenderableObjects(vector<vector<Graphics::DecoratedGraphicsObject*>> input);
	virtual void setRenderableObjects(vector<vector<Graphics::DecoratedGraphicsObject*>> input, string signature) {};
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, int programIndex);
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, int programIndex, string signature) {};
	virtual void setProbe(string passSignature, string frameBufferSignature);
	virtual void setFrameBuffer(DecoratedFrameBuffer* fb);
	virtual void setTextureUniforms(int index);
};

class GeometryPass : public RenderPass
{
protected:
	void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(void);
	virtual void setupObjectwiseUniforms(int programIndex, int index);
public:
	GeometryPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, DecoratedFrameBuffer* frameBuffer = nullptr, bool terminal = false);
	GeometryPass(vector<ShaderProgramPipeline*> shaderPipelines, DecoratedFrameBuffer* frameBuffer = nullptr, bool terminal = false);
	~GeometryPass() {};
	virtual void setupCamera(Camera* cam);
	virtual void setupOnHover(unsigned int id);
};

class LightPass : public RenderPass
{
protected:
	void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(void);
public:
	LightPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, bool terminal = false);
	LightPass(vector<ShaderProgramPipeline*> shaderPipelines, bool terminal = false);
	~LightPass() {};
};

