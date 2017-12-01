#include "Pass.h"
#include "WindowContext.h"
#include "Camera.h"

Pass::Pass()
{
}

Pass::Pass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature) : 
	DirectedGraphNode(neighbors, signature), shaderPipelines(shaderPipelines)
{
	registerUniforms();
}

Pass::Pass(vector<ShaderProgramPipeline*> shaderPipelines, string signature) : DirectedGraphNode(signature), shaderPipelines(shaderPipelines)
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
	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		uniformIDs.push_back(vector<tuple<string, GLuint, GLuint, UniformType>>());

		for (int j = 0; j < shaderPipelines[i]->attachedPrograms.size(); j++)
		{
			auto p = shaderPipelines[i]->attachedPrograms[j];
			for (int k = 0; k < p->uniformIDs.size(); k++)
			{
				uniformIDs[i].push_back(tuple<string, GLuint, GLuint, UniformType>(get<0>(p->uniformIDs[k]), p->program, get<1>(p->uniformIDs[k]), get<2>(p->uniformIDs[k])));
			}
		}
	}
}

int Pass::getUniformIndexBySignature(int index, string signature)
{
	for (int j = 0; j < uniformIDs[index].size(); j++)
	{
		if (get<0>(uniformIDs[index][j]) == signature)
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

RenderPass::RenderPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipelines, neighbors, signature), frameBuffer(frameBuffer), terminal(terminal)
{
	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		renderableObjects.push_back(vector<DecoratedGraphicsObject*>());
		floatTypeUniformPointers.push_back(vector<tuple<int, GLfloat*>>());
		uintTypeUniformPointers.push_back(vector<tuple<int, GLuint*>>());
		intTypeUniformPointers.push_back(vector<tuple<int, GLint*>>());
		uintTypeUniformValues.push_back(vector<tuple<int, GLuint>>());
	}
}

RenderPass::RenderPass(vector<ShaderProgramPipeline*> shaderPipelines, string signature, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	Pass(shaderPipelines, signature), frameBuffer(frameBuffer), terminal(terminal)
{
	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		renderableObjects.push_back(vector<DecoratedGraphicsObject*>());
		floatTypeUniformPointers.push_back(vector<tuple<int, GLfloat*>>());
		uintTypeUniformPointers.push_back(vector<tuple<int, GLuint*>>());
		intTypeUniformPointers.push_back(vector<tuple<int, GLint*>>());
		uintTypeUniformValues.push_back(vector<tuple<int, GLuint>>());
	}
}

RenderPass::~RenderPass()
{

}

void RenderPass::initFrameBuffers(void)
{
}

void RenderPass::clearRenderableObjects(int index)
{
	renderableObjects[index].clear();
}

void RenderPass::setRenderableObjects(vector<vector<DecoratedGraphicsObject*>> input)
{
	renderableObjects = input;
}

void RenderPass::addRenderableObjects(DecoratedGraphicsObject* input, int programIndex)
{
	renderableObjects[programIndex].push_back(input);
}

void RenderPass::setProbe(string passSignature, string fbSignature)
{

}

void RenderPass::setFrameBuffer(DecoratedFrameBuffer* fb)
{
	frameBuffer = fb;
}

void RenderPass::setTextureUniforms(int index)
{
	for (int i = 0, count = 0; i < uniformIDs[index].size(); i++)
	{
		if (get<3>(uniformIDs[index][i]) == TEXTURE)
		{
			glProgramUniform1iv(get<1>(uniformIDs[index][i]), get<2>(uniformIDs[index][i]), 1, &count);
			count++;
		}
	}

	// IMPLEMENT FOR OTHER DATA TYPES
	for (int i = 0; i < floatTypeUniformPointers[index].size(); i++)
	{
		auto uID = uniformIDs[index][get<0>(floatTypeUniformPointers[index][i])];

		if (get<3>(uID) == MATRIX4FV)
		{
			glProgramUniformMatrix4fv(get<1>(uID), get<2>(uID), 1, GL_FALSE, get<1>(floatTypeUniformPointers[index][i]));
		}
	}

	for (int i = 0; i < uintTypeUniformValues[index].size(); i++)
	{
		auto uID = uniformIDs[index][get<0>(uintTypeUniformValues[index][i])];

		if (get<3>(uID) == ONEUI)
		{
			glProgramUniform1ui(get<1>(uID), get<2>(uID), get<1>(uintTypeUniformValues[index][i]));
		}
	}
}

void RenderPass::clearBuffers(void)
{

}

void RenderPass::configureGL(void)
{

}

void RenderPass::renderObjects(int programIndex)
{
	for (int i = 0; i < renderableObjects[programIndex].size(); i++)
	{
		setupObjectwiseUniforms(programIndex, i);
		renderableObjects[programIndex][i]->enableBuffers();
		renderableObjects[programIndex][i]->draw();
	}
}

void RenderPass::setupObjectwiseUniforms(int programIndex, int index)
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

	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		// Shader setup
		shaderPipelines[i]->use();

		// Texture uniforms setup
		setTextureUniforms(i);

		// Object rendering
		renderObjects(i);
	}
}

GeometryPass::GeometryPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	RenderPass(shaderPipelines, neighbors, "GEOMETRYPASS", frameBuffer)
{
	initFrameBuffers();
}

GeometryPass::GeometryPass(vector<ShaderProgramPipeline*> shaderPipelines, DecoratedFrameBuffer* frameBuffer, bool terminal) :
	RenderPass(shaderPipelines, "GEOMETRYPASS", frameBuffer)
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
//	glEnable(GL_CULL_FACE);
}

void GeometryPass::setupObjectwiseUniforms(int programIndex, int index)
{
	GLuint p = shaderPipelines[programIndex]->getProgramByEnum(GL_VERTEX_SHADER)->program;
	glProgramUniformMatrix4fv(p, glGetUniformLocation(p, "Model"), 1, GL_FALSE, &(renderableObjects[programIndex][index]->getModelMatrix()[0][0]));
}

void GeometryPass::setupCamera(Camera* cam)
{
	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		updatePointerBySignature<float>(i, "Projection", &(cam->Projection[0][0]));
		updatePointerBySignature<float>(i, "View", &(cam->View[0][0]));
	}
}

void GeometryPass::setupOnHover(unsigned int id)
{
	for (int i = 0; i < shaderPipelines.size(); i++)
	{
		updateValueBySignature<unsigned int>(i, "selectedRef", id);
	}
}

LightPass::LightPass(vector<ShaderProgramPipeline*> shaderPipelines, vector<Pass*> neighbors, bool terminal) : 
	RenderPass(shaderPipelines, neighbors, "LIGHTPASS", nullptr)
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

LightPass::LightPass(vector<ShaderProgramPipeline*> shaderPipelines, bool terminal) : 
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

void LightPass::configureGL(void)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}