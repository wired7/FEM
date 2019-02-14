#include "Pass.h"
#include "WindowContext.h"
#include "Camera.h"
#include <typeinfo>
#include <typeindex>

using namespace Graphics;
Pass::Pass()
{
}

Pass::Pass(map<string, ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature) : 
	DirectedGraphNode(neighbors, signature), shaderPipelines(shaderPipelines)
{
	registerUniforms();
}

Pass::Pass(map<string, ShaderProgramPipeline*> shaderPipelines, string signature) : DirectedGraphNode(signature), shaderPipelines(shaderPipelines)
{
	registerUniforms();
}

Pass::~Pass()
{
}

void Pass::execute(void)
{
	currentCount++;
	if (currentCount < incomingCount)
	{
		return;
	}

	currentCount = 0;

	executeOwnBehaviour();

	for (int i = 0; i < neighbors.size(); i++)
	{
		neighbors[i]->execute();
	}
}

void Pass::registerUniforms(void)
{
	for (const auto& pipeline : shaderPipelines)
	{
		uniformIDs[pipeline.second->signature] = vector<tuple<string, GLuint, GLuint, UniformType>>();

		for (const auto& program : pipeline.second->attachedPrograms)
		{
			for (auto& uniformID : program->uniformIDs)
			{
				uniformIDs[pipeline.second->signature].push_back(tuple<string, GLuint, GLuint, UniformType>(get<0>(uniformID),
																											program->program,
																											get<1>(uniformID),
																											get<2>(uniformID)));
			}
		}
	}
}

int Pass::getUniformIndexBySignature(const std::string& programSignature, string signature)
{
	for (int j = 0; j < uniformIDs[programSignature].size(); j++)
	{
		if (get<0>(uniformIDs[programSignature][j]) == signature)
		{
			return j;
		}
	}

	return -1;
}

void Pass::addNeighbor(Pass* neighbor)
{
	DirectedGraphNode::addNeighbor(neighbor);
	neighbor->incomingCount++;
}

void CLPass::addBuffer(AbstractCLBuffer* buffer, const std::string& bufferSignature, const std::string& kernelSignature, int index, bool isGLBuffer)
{
	std::map<std::string, std::map<std::string, std::pair<AbstractCLBuffer*, int>>>* targetMap = &buffers;

	if (isGLBuffer)
	{
		targetMap = &clGLBuffers;
	}

	(*targetMap)[kernelSignature][bufferSignature] = std::make_pair(buffer, index);
}

void CLPass::executeOwnBehaviour(void)
{
	glFinish();

	for (const auto& clKernel : clKernels)
	{
		for (const auto& buffer : buffers[clKernel.first])
		{
			buffer.second.first->enableBuffer(clKernel.second.first, buffer.second.second);
		}

		for (const auto& clGLBuffer : clGLBuffers[clKernel.first])
		{
			clGLBuffer.second.first->enableBuffer(clKernel.second.first, clGLBuffer.second.second);
			clEnqueueAcquireGLObjects(clKernel.second.first->context->commandQueues[0], 1, &clGLBuffer.second.first->bufferPointer, 0, 0, 0);
		}

		clKernel.second.first->execute(clKernel.second.second.first, clKernel.second.second.second);

		clFinish(clKernel.second.first->context->commandQueues[0]);

		for (const auto& clGLBuffer : clGLBuffers[clKernel.first])
		{
			clEnqueueReleaseGLObjects(clKernel.second.first->context->commandQueues[0], 1, &clGLBuffer.second.first->bufferPointer, 0, 0, 0);
		}

		clFinish(clKernel.second.first->context->commandQueues[0]);
	}
}

RenderPass::RenderPass(map<string, ShaderProgramPipeline*> shaderPipelines, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipelines, signature), frameBuffer(frameBuffer), terminal(terminal)
{
	for (const auto& pipeline : shaderPipelines)
	{
		renderableObjects[pipeline.second->signature] = vector<DecoratedGraphicsObject*>();
		floatTypeUniformPointers[pipeline.second->signature] = vector<tuple<int, GLfloat*>>();
		uintTypeUniformPointers[pipeline.second->signature] = vector<tuple<int, GLuint*>>();
		intTypeUniformPointers[pipeline.second->signature] = vector<tuple<int, GLint*>>();
		uintTypeUniformValues[pipeline.second->signature] = vector<tuple<int, GLuint>>();
	}
}

RenderPass::~RenderPass()
{

}

void RenderPass::initFrameBuffers(void)
{
}

void RenderPass::clearRenderableObjects(const std::string& signature)
{
	renderableObjects[signature].clear();
}

void RenderPass::setRenderableObjects(map<std::string, vector<DecoratedGraphicsObject*>> input)
{
	renderableObjects = input;
}

void RenderPass::addRenderableObjects(DecoratedGraphicsObject* input, const std::string& programSignature)
{
	renderableObjects[programSignature].push_back(input);
}

void RenderPass::setProbe(const std::string& passSignature, const std::string& fbSignature)
{
}

void RenderPass::setFrameBuffer(DecoratedFrameBuffer* fb)
{
	frameBuffer = fb;
}

void RenderPass::setTextureUniforms(const std::string& programSignature)
{
	for (int i = 0, count = 0; i < uniformIDs[programSignature].size(); i++)
	{
		if (get<3>(uniformIDs[programSignature][i]) == TEXTURE)
		{
			glProgramUniform1iv(get<1>(uniformIDs[programSignature][i]), get<2>(uniformIDs[programSignature][i]), 1, &count);
			count++;
		}
	}

	// IMPLEMENT FOR OTHER DATA TYPES
	for (int i = 0; i < floatTypeUniformPointers[programSignature].size(); i++)
	{
		auto uID = uniformIDs[programSignature][get<0>(floatTypeUniformPointers[programSignature][i])];

		if (get<3>(uID) == MATRIX4FV)
		{
			glProgramUniformMatrix4fv(get<1>(uID), get<2>(uID), 1, GL_FALSE, get<1>(floatTypeUniformPointers[programSignature][i]));
		}
		else if (get<3>(uID) == VECTOR4FV)
		{
			glProgramUniform4fv(get<1>(uID), get<2>(uID), 1, get<1>(floatTypeUniformPointers[programSignature][i]));
		}
	}

	for (int i = 0; i < uintTypeUniformValues[programSignature].size(); i++)
	{
		auto uID = uniformIDs[programSignature][get<0>(uintTypeUniformValues[programSignature][i])];

		if (get<3>(uID) == ONEUI)
		{
			glProgramUniform1ui(get<1>(uID), get<2>(uID), get<1>(uintTypeUniformValues[programSignature][i]));
		}
	}
}

void RenderPass::clearBuffers(void)
{
}

void RenderPass::configureGL(const std::string& programSignature)
{
}

void RenderPass::renderObjects(const std::string& programSignature)
{
	for (int i = 0; i < renderableObjects[programSignature].size(); i++)
	{
		setupObjectwiseUniforms(programSignature, i);
		renderableObjects[programSignature][i]->enableBuffers();
		renderableObjects[programSignature][i]->draw();
	}
}

void RenderPass::setupObjectwiseUniforms(const std::string& programSignature, int index)
{
}

void RenderPass::executeOwnBehaviour()
{
	// Set input textures from incoming passes for this stage
	for (int i = 0, count = 0; i < parents.size(); i++)
	{
		auto parent = dynamic_cast<RenderPass*>(parents[i]);

		if (parent == nullptr)
		{
			continue;
		}

		count += parent->frameBuffer->bindTexturesForPass(count);
	}

	// Set output textures
	frameBuffer->drawBuffers();

	// Clear buffers
	if (clearBuff)
	{
		clearBuffers();
	}

	for (const auto pipeline : shaderPipelines)
	{
		// GL configuration
		configureGL(pipeline.second->signature);

		// Shader setup
		pipeline.second->use();

		// Texture uniforms setup
		setTextureUniforms(pipeline.second->signature);

		// Object rendering
		renderObjects(pipeline.second->signature);
	}
}

GeometryPass::GeometryPass(map<string, ShaderProgramPipeline*> shaderPipelines,
			  std::string signature,
			  DecoratedFrameBuffer* frameBuffer,
			  int pickingBufferCount,
			  int stencilBufferCount,
			  bool terminal) :
	pickingBufferCount(pickingBufferCount), stencilBufferCount(stencilBufferCount), RenderPass(shaderPipelines, signature, frameBuffer, terminal)
{
	initFrameBuffers();
}

void GeometryPass::initFrameBuffers(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	auto colorsBuffer = new ImageFrameBuffer(width, height, "COLORS");
	auto normalsBuffer = new ImageFrameBuffer(colorsBuffer, width, height, "NORMALS");
	DecoratedFrameBuffer* lastBuffer = new ImageFrameBuffer(normalsBuffer, width, height, "POSITIONS");

	for (int i = 0; i < pickingBufferCount; ++i)
	{
		lastBuffer = new PickingBuffer(lastBuffer, width, height, "PICKING" + std::to_string(i));
	}

	for (int i = 0; i < stencilBufferCount; ++i)
	{
		lastBuffer = new PickingBuffer(lastBuffer, width, height, "STENCIL" + std::to_string(i));
	}

	frameBuffer = lastBuffer;
}


void GeometryPass::clearBuffers(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GeometryPass::configureGL(const std::string& programSignature)
{
	if (!shaderPipelines[programSignature]->alphaRendered)
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_MULTISAMPLE);

//	glEnable(GL_CULL_FACE);
}

void GeometryPass::setupObjectwiseUniforms(const std::string& programSignature, int index)
{
	GLuint p = shaderPipelines[programSignature]->getProgramByEnum(GL_VERTEX_SHADER)->program;
	glProgramUniformMatrix4fv(p, glGetUniformLocation(p, "Model"), 1, GL_FALSE, &(renderableObjects[programSignature][index]->getModelMatrix()[0][0]));
}

void GeometryPass::setupCamera(Camera* cam)
{
	for (const auto pipeline : shaderPipelines)
	{
		updatePointerBySignature<float>(pipeline.second->signature, "Projection", &(cam->Projection[0][0]));
		updatePointerBySignature<float>(pipeline.second->signature, "View", &(cam->View[0][0]));
	}
}

void GeometryPass::setupVec4f(vec4& input, string name)
{
	for (const auto pipeline : shaderPipelines)
	{
		updatePointerBySignature<float>(pipeline.second->signature, name, &(input[0]));
	}
}

void GeometryPass::setupOnHover(unsigned int id)
{
	for (const auto pipeline : shaderPipelines)
	{
		updateValueBySignature<unsigned int>(pipeline.second->signature, "selectedRef", id);
	}
}

/*void StenciledGeometryPass::initFrameBuffers(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	auto colorsBuffer = new ImageFrameBuffer(width, height, "COLORS");
	auto normalsBuffer = new ImageFrameBuffer(colorsBuffer, width, height, "NORMALS");
	auto positionsBuffer = new ImageFrameBuffer(normalsBuffer, width, height, "POSITIONS");
	auto pickingBuffer = new PickingBuffer(positionsBuffer, width, height, "PICKING");

	frameBuffer = pickingBuffer;
}


void StenciledGeometryPass::clearBuffers(void)
{
	GeometryPass::clearBuffers();
	glClear(GL_STENCIL_BUFFER_BIT);
}

void StenciledGeometryPass::configureGL(const std::string& programSignature)
{
	GeometryPass::configureGL(programSignature);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
}*/

LightPass::LightPass(map<string, ShaderProgramPipeline*> shaderPipelines, bool terminal) : 
	RenderPass(shaderPipelines, "LIGHTPASS", nullptr)
{
	if (terminal)
	{
		frameBuffer = new DefaultFrameBuffer();
	}
	else
	{
		initFrameBuffers();
	}
}

void LightPass::initFrameBuffers(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	auto imageFB = new ImageFrameBuffer(width, height, "MAINIMAGE");

	frameBuffer = imageFB;
}

void LightPass::clearBuffers(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LightPass::configureGL(const std::string& programSignature)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}