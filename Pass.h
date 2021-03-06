#pragma once
#include <vector>
#include <map>
#include <glew.h>
#include <glfw3.h>
#include <utility>
#include "GeometricalMeshObjects.h"
#include "DirectedGraphNode.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Context.h"
#include "OpenCLContext.h"

using namespace std;


class Pass : public DirectedGraphNode<Pass>
{
protected:
	int incomingCount = 0;
	int currentCount = 0;
	map<string, ShaderProgramPipeline*> shaderPipelines;
	virtual void executeOwnBehaviour() = 0;
public:
	map<string, vector<tuple<string, GLuint, GLuint, UniformType>>> uniformIDs;
	map<string, vector<tuple<int, GLfloat*>>> floatTypeUniformPointers;
	map<string, vector<tuple<int, GLuint*>>> uintTypeUniformPointers;
	map<string, vector<tuple<int, GLint*>>> intTypeUniformPointers;
	map<string, vector<tuple<int, GLuint>>> uintTypeUniformValues;
	Pass();
	Pass(map<string, ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature);
	Pass(map<string, ShaderProgramPipeline*> shaderPipelines, string signature);
	~Pass();
	virtual void execute(void);
	virtual void registerUniforms(void);
	virtual int getUniformIndexBySignature(const std::string& programSignature, string signature);
	template<typename T> void updatePointerBySignature(const std::string& programSignature, string signature, T* pointer);
	template<typename T> void updateValueBySignature(const std::string& programSignature, string signature, T value);
	virtual void addNeighbor(Pass* neighbor);
};

// IMPLEMENT FOR OTHER TYPES!!!
template<typename T> void Pass::updatePointerBySignature(const std::string& programSignature, string signature, T* pointer)
{
	if (is_same<T, GLfloat>::value || is_same<T, float>::value)
	{
		for (int i = 0; i < floatTypeUniformPointers[programSignature].size(); i++)
		{
			if (get<0>(uniformIDs[programSignature][get<0>(floatTypeUniformPointers[programSignature][i])]) == signature)
			{
				get<1>(floatTypeUniformPointers[programSignature][i]) = pointer;
				return;
			}
		}

		floatTypeUniformPointers[programSignature].push_back(tuple<int, T*>(getUniformIndexBySignature(programSignature, signature), pointer));
	}
}

template<typename T> void Pass::updateValueBySignature(const std::string& programSignature, string signature, T value)
{
	if (is_same<T, GLuint>::value || is_same<T, unsigned int>::value)
	{
		for (int i = 0; i < uintTypeUniformValues[programSignature].size(); i++)
		{
			if (get<0>(uniformIDs[programSignature][get<0>(uintTypeUniformValues[programSignature][i])]) == signature)
			{
				get<1>(uintTypeUniformValues[programSignature][i]) = value;
				return;
			}
		}

		uintTypeUniformValues[programSignature].push_back(tuple<int, T>(getUniformIndexBySignature(programSignature, signature), value));
	}
}

class CLPass : public Pass
{
protected:
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>> buffers;
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>> clGLBuffers;
	std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> clKernels;
	virtual void executeOwnBehaviour(void);
public:
	CLPass(std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> kernels) : clKernels(kernels) {};
	~CLPass() {};
	void addBuffer(AbstractCLBuffer* buffer, const std::string& bufferSignature, const std::string& kernelSignature, int index, bool isGLBuffer = false);
};

class RenderPass : public Pass
{
protected:
	map<std::string, vector<Graphics::DecoratedGraphicsObject*>> renderableObjects;
	bool terminal;
	virtual void initFrameBuffers(void) = 0;
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
	virtual void renderObjects(const std::string& programSignature);
	virtual void setupObjectwiseUniforms(const std::string& programSignature, int index);
	virtual void executeOwnBehaviour(void);
public:
	bool clearBuff = true;
	DecoratedFrameBuffer* frameBuffer;
	RenderPass(map<string, ShaderProgramPipeline*> shaderPipelines, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal = false);
	~RenderPass();
	// TODO: Give the user the ability to apply renderable objects to any inner pass for rendering before the call
	virtual void clearRenderableObjects(const std::string& programSignature);
	virtual void clearRenderableObjects(const std::string& programSignature, const string& objectSignature) {};
	virtual void setRenderableObjects(map<std::string, vector<Graphics::DecoratedGraphicsObject*>> input);
	virtual void setRenderableObjects(vector<vector<Graphics::DecoratedGraphicsObject*>> input, string signature) {};
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& programSignature);
	virtual void addRenderableObjects(Graphics::DecoratedGraphicsObject* input, const std::string& programSignature, const string& objectSignature) {};
	virtual void setProbe(const std::string& passSignature, const std::string& frameBufferSignature);
	virtual void setFrameBuffer(DecoratedFrameBuffer* fb);
	virtual void setTextureUniforms(const std::string& programSignature);
};

class GeometryPass : public RenderPass
{
protected:
	virtual void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
	virtual void setupObjectwiseUniforms(const std::string& programSignature, int index);
public:
	GeometryPass(map<string, ShaderProgramPipeline*> shaderPipelines,
				 std::string signature = "GEOMETRYPASS",
				 DecoratedFrameBuffer* frameBuffer = nullptr,
				 int pickingBufferCount = 0,
				 int stencilBufferCount = 0,
				 bool terminal = false);
	~GeometryPass() {};
	virtual void setupCamera(Camera* cam);
	virtual void setupOnHover(unsigned int id);
	virtual void setupVec4f(glm::vec4& input, std::string name);

	int pickingBufferCount;
	int stencilBufferCount;
};

/*class StenciledGeometryPass : public GeometryPass
{
protected:
	virtual void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
};*/

class LightPass : public RenderPass
{
protected:
	virtual void initFrameBuffers(void);
	virtual void clearBuffers(void);
	virtual void configureGL(const std::string& programSignature);
public:
	LightPass(map<string, ShaderProgramPipeline*> shaderPipelines, bool terminal = false);
	~LightPass() {};
};

