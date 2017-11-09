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
	ShaderProgramPipeline* shaderPipeline;
	virtual void executeOwnBehaviour() = 0;
public:
	vector<tuple<string, GLuint, GLuint, UniformType>> uniformIDs;
	vector<tuple<int, GLfloat*>> floatTypeUniformPointers;
	vector<tuple<int, GLuint*>> uintTypeUniformPointers;
	vector<tuple<int, GLint*>> intTypeUniformPointers;
	vector<tuple<int, GLuint>> uintTypeUniformValues;
	Pass();
	Pass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, string signature);
	Pass(ShaderProgramPipeline* shaderPipeline, string signature);
	~Pass();
	virtual void execute(void);
	virtual void registerUniforms(void);
	virtual int getUniformIndexBySignature(string signature);
	template<typename T> void updatePointerBySignature(string signature, T* pointer);
	template<typename T> void updateValueBySignature(string signature, T value);
	virtual void addNeighbor(Pass* neighbor);
};

// IMPLEMENT FOR OTHER TYPES!!!
template<typename T> void Pass::updatePointerBySignature(string signature, T* pointer)
{
	if (is_same<T, GLfloat>::value || is_same<T, float>::value)
	{
		for (int i = 0; i < floatTypeUniformPointers.size(); i++)
		{
			if (get<0>(uniformIDs[get<0>(floatTypeUniformPointers[i])]) == signature)
			{
				get<1>(floatTypeUniformPointers[i]) = pointer;
				return;
			}
		}

		floatTypeUniformPointers.push_back(tuple<int, T*>(getUniformIndexBySignature(signature), pointer));
	}
}

template<typename T> void Pass::updateValueBySignature(string signature, T value)
{
	if (is_same<T, GLuint>::value || is_same<T, unsigned int>::value)
	{
		for (int i = 0; i < uintTypeUniformValues.size(); i++)
		{
			if (get<0>(uniformIDs[get<0>(uintTypeUniformValues[i])]) == signature)
			{
				get<1>(uintTypeUniformValues[i]) = value;
				return;
			}
		}

		uintTypeUniformValues.push_back(tuple<int, T>(getUniformIndexBySignature(signature), value));
	}
}

class RenderPass : public Pass
{
protected:
	vector<DecoratedGraphicsObject*> renderableObjects;
	bool terminal;
	virtual void initFrameBuffers(void) = 0;
	virtual void clearBuffers(void);
	virtual void configureGL(void);
	virtual void renderObjects(void);
	virtual void setupObjectwiseUniforms(int index);
	virtual void executeOwnBehaviour(void);
public:
	DecoratedFrameBuffer* frameBuffer;
	RenderPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	RenderPass(ShaderProgramPipeline* shaderPipeline, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	~RenderPass();
	// TODO: Give the user the ability to apply renderable objects to any inner pass for rendering before the call
	virtual void clearRenderableObjects(void);
	virtual void clearRenderableObjects(string signature) {};
	virtual void setRenderableObjects(vector<DecoratedGraphicsObject*> input);
	virtual void setRenderableObjects(vector<DecoratedGraphicsObject*> input, string signature) {};
	virtual void addRenderableObjects(DecoratedGraphicsObject* input);
	virtual void addRenderableObjects(DecoratedGraphicsObject* input, string signature) {};
	virtual void setProbe(string passSignature, string frameBufferSignature);
	virtual void setFrameBuffer(DecoratedFrameBuffer* fb);
	virtual void setTextureUniforms();
};

class GeometryPass : public RenderPass
{
protected:
	void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(void);
	virtual void setupObjectwiseUniforms(int index);
public:
	GeometryPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, DecoratedFrameBuffer* frameBuffer = nullptr, bool terminal = false);
	GeometryPass(ShaderProgramPipeline* shaderPipeline, DecoratedFrameBuffer* frameBuffer = nullptr, bool terminal = false);
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
	LightPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, bool terminal = false);
	LightPass(ShaderProgramPipeline* shaderPipeline, bool terminal = false);
	~LightPass() {};
};

