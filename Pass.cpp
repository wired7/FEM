#include "Pass.h"
#include "WindowContext.h"
#include "Camera.h"

Pass::Pass()
{
}

Pass::Pass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, string signature) : DirectedGraphNode(neighbors, signature), shaderPipeline(shaderPipeline)
{
	registerUniforms();
}

Pass::Pass(ShaderProgramPipeline* shaderPipeline, string signature) : DirectedGraphNode(signature), shaderPipeline(shaderPipeline)
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
	if (shaderPipeline != nullptr)
	{
		uniformIDs.clear();

		for (int i = 0; i < shaderPipeline->attachedPrograms.size(); i++)
		{
			auto p = shaderPipeline->attachedPrograms[i];
			for (int j = 0; j < p->uniformIDs.size(); j++)
			{
				uniformIDs.push_back(tuple<string, GLuint, GLuint, UniformType>(get<0>(p->uniformIDs[j]), p->program, get<1>(p->uniformIDs[j]), get<2>(p->uniformIDs[j])));
			}
		}
	}
}

int Pass::getUniformIndexBySignature(string signature)
{
	for (int i = 0; i < uniformIDs.size(); i++)
	{
		if (get<0>(uniformIDs[i]) == signature)
		{
			return i;
		}
	}

	return -1;
}

void Pass::addNeighbor(Pass* neighbor)
{
	DirectedGraphNode::addNeighbor(neighbor);
	neighbor->incomingCount++;
}

RenderPass::RenderPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipeline, neighbors, signature), frameBuffer(frameBuffer), terminal(terminal)
{

}

RenderPass::RenderPass(ShaderProgramPipeline* shaderPipeline, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipeline, signature), frameBuffer(frameBuffer), terminal(terminal)
{

}

RenderPass::~RenderPass()
{

}

void RenderPass::initFrameBuffers(void)
{
}

void RenderPass::clearRenderableObjects(void)
{
	renderableObjects.clear();
}

void RenderPass::setRenderableObjects(vector<DecoratedGraphicsObject*> input)
{
	renderableObjects = input;
}

void RenderPass::addRenderableObjects(DecoratedGraphicsObject* input)
{
	renderableObjects.push_back(input);
}

void RenderPass::setProbe(string passSignature, string fbSignature)
{

}

void RenderPass::setFrameBuffer(DecoratedFrameBuffer* fb)
{
	frameBuffer = fb;
}

void RenderPass::setTextureUniforms(void)
{
	for (int i = 0, count = 0; i < uniformIDs.size(); i++)
	{
		if (get<3>(uniformIDs[i]) == TEXTURE)
		{
			glProgramUniform1iv(get<1>(uniformIDs[i]), get<2>(uniformIDs[i]), 1, &count);
			count++;
		}
	}

	// IMPLEMENT FOR OTHER DATA TYPES
	for (int i = 0; i < floatTypeUniformPointers.size(); i++)
	{
		auto uID = uniformIDs[get<0>(floatTypeUniformPointers[i])];

		if (get<3>(uID) == MATRIX4FV)
		{
			glProgramUniformMatrix4fv(get<1>(uID), get<2>(uID), 1, GL_FALSE, get<1>(floatTypeUniformPointers[i]));
		}
	}

	for (int i = 0; i < uintTypeUniformValues.size(); i++)
	{
		auto uID = uniformIDs[get<0>(uintTypeUniformValues[i])];

		if (get<3>(uID) == ONEUI)
		{
			glProgramUniform1ui(get<1>(uID), get<2>(uID), get<1>(uintTypeUniformValues[i]));
		}
	}
}

void RenderPass::clearBuffers(void)
{

}

void RenderPass::configureGL(void)
{

}

void RenderPass::renderObjects(void)
{
	for (int i = 0; i < renderableObjects.size(); i++)
	{
		setupObjectwiseUniforms(i);
		renderableObjects[i]->enableBuffers();
		renderableObjects[i]->draw();
	}
}

void RenderPass::setupObjectwiseUniforms(int index)
{

}

void RenderPass::executeOwnBehaviour()
{
	// Set input textures from incoming passes for this stage
	for (int i = 0, count = 0; i < parents.size(); i++)
	{
		count += ((RenderPass*)parents[i])->frameBuffer->bindTexturesForPass(count);
	}

	// Set output textures
	frameBuffer->drawBuffers();

	// Clear buffers
	clearBuffers();

	// GL configuration
	configureGL();

	// Shader setup
	shaderPipeline->use();

	// Texture uniforms setup
	setTextureUniforms();

	// Object rendering
	renderObjects();
}

GeometryPass::GeometryPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	RenderPass(shaderPipeline, neighbors, "GEOMETRYPASS", frameBuffer)
{
	initFrameBuffers();
}

GeometryPass::GeometryPass(ShaderProgramPipeline* shaderPipeline, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	RenderPass(shaderPipeline, "GEOMETRYPASS", frameBuffer)
{
	initFrameBuffers();
}

void GeometryPass::initFrameBuffers(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	auto colorsBuffer = new ImageFrameBuffer(width, height, "COLORS");
	auto normalsBuffer = new ImageFrameBuffer(colorsBuffer, width, height, "NORMALS");
	auto positionsBuffer = new ImageFrameBuffer(normalsBuffer, width, height, "POSITIONS");
	auto pickingBuffer = new PickingBuffer(positionsBuffer, width, height, "PICKING");

	frameBuffer = pickingBuffer;
}


void GeometryPass::clearBuffers(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GeometryPass::configureGL(void)
{
	glEnable(GL_DEPTH_TEST);
}

void GeometryPass::setupObjectwiseUniforms(int index)
{
	GLuint p = shaderPipeline->getProgramByEnum(GL_VERTEX_SHADER)->program;
	glProgramUniformMatrix4fv(p, glGetUniformLocation(p, "Model"), 1, GL_FALSE, &(renderableObjects[index]->getModelMatrix()[0][0]));
}

void GeometryPass::setupCamera(Camera* cam)
{
	updatePointerBySignature<float>("Projection", &(cam->Projection[0][0]));
	updatePointerBySignature<float>("View", &(cam->View[0][0]));
}

void GeometryPass::setupOnHover(unsigned int id)
{
	updateValueBySignature<unsigned int>("selectedRef", id);
}

LightPass::LightPass(ShaderProgramPipeline* shaderPipeline, vector<Pass*> neighbors, bool terminal) : RenderPass(shaderPipeline, neighbors, "LIGHTPASS", nullptr)
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

LightPass::LightPass(ShaderProgramPipeline* shaderPipeline, bool terminal) : RenderPass(shaderPipeline, "LIGHTPASS", nullptr)
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

void LightPass::configureGL(void)
{
	glDisable(GL_DEPTH_TEST);
}