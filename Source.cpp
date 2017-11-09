#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h> 
#include "WindowContext.h"
#include "ShaderProgramPipeline.h"
#include "Context.h"

using namespace std;
using namespace glm;

// GLEW and GLFW initialization. Projection and View matrix setup
int init() {

	WindowContext(800, 600, "VISUAL COMPUTING");

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//Check version of OpenGL and print
	std::printf("*** OpenGL Version: %s ***\n", glGetString(GL_VERSION));

	// Enable depth test
	glDepthFunc(GL_LESS);

	glPointSize(3.0f);

	srand(time(NULL));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\gPass.VERTEXSHADER", { tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),  tuple<const GLchar*, UniformType>("Model", MATRIX4FV), tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "GPASSVS");
	
	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\lightPass.VERTEXSHADER", {}, "LPASSVS");
	
	ShaderProgram::getShaderProgram<VertexShaderProgram>("shaders\\pointGPass.VERTEXSHADER", { tuple<const GLchar*, UniformType>("View", MATRIX4FV),
		tuple<const GLchar*, UniformType>("Projection", MATRIX4FV),  tuple<const GLchar*, UniformType>("Model", MATRIX4FV), tuple<const GLchar*, UniformType>("selectedRef", ONEUI) }, "GPASSVS2");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\gPass.FRAGMENTSHADER", {}, "GPASSFS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\lightPass.FRAGMENTSHADER", { tuple<const GLchar*, UniformType>("gColors", TEXTURE),
		tuple<const GLchar*, UniformType>("gNormals", TEXTURE), tuple<const GLchar*, UniformType>("gPositions", TEXTURE) }, "LPASSFS");

	ShaderProgram::getShaderProgram<FragmentShaderProgram>("shaders\\pointGPass.FRAGMENTSHADER", {}, "GPASSFS2");

	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A"));
	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("A"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("LPASSVS")->attachToPipeline(ShaderProgramPipeline::getPipeline("B"));
	ShaderProgram::getShaderProgram<VertexShaderProgram>("LPASSFS")->attachToPipeline(ShaderProgramPipeline::getPipeline("B"));

	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSVS2")->attachToPipeline(ShaderProgramPipeline::getPipeline("C"));
	ShaderProgram::getShaderProgram<VertexShaderProgram>("GPASSFS2")->attachToPipeline(ShaderProgramPipeline::getPipeline("C"));

	return 1;
}

int main()
{
	if (init() < 0)
		return -1;

	SurfaceViewContext* context1 = new SurfaceViewContext();

	do {
		AbstractContext::activeContext->update();
		// Swap buffers
		glfwSwapBuffers(WindowContext::window);
		glfwPollEvents();

	} while (glfwWindowShouldClose(WindowContext::window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}