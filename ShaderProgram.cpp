#include "ShaderProgram.h"
#include <iostream>
#include <fstream>
#include <string>

vector<ShaderProgram*> ShaderProgram::compiledPrograms;

ShaderProgram* ShaderProgram::getCompiledProgram(string filePath)
{
	for (int i = 0; i < compiledPrograms.size(); i++)
	{
		if (compiledPrograms[i]->filePath == filePath)
		{
			return compiledPrograms[i];
		}
	}

	return nullptr;
}

ShaderProgram* ShaderProgram::getCompiledProgramBySignature(string signature)
{
	for (int i = 0; i < compiledPrograms.size(); i++)
	{
		if (compiledPrograms[i]->signature == signature)
		{
			return compiledPrograms[i];
		}
	}

	return nullptr;
}


ShaderProgram::ShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature, GLenum shader, GLenum shaderBit) : 
	filePath(filePath), signature(signature), shader(shader), shaderBit(shaderBit)
{
	loadShaderProgram();
	bindShaderProgram();

	for (int i = 0; i < uIDs.size(); i++)
	{
		uniformIDs.push_back(tuple<string, GLint, UniformType>(get<0>(uIDs[i]), glGetUniformLocation(program, get<0>(uIDs[i])), get<1>(uIDs[i])));
	}
}

ShaderProgram::~ShaderProgram()
{
	glGetError();
	cout << "DELETING SHADER PROGRAM " << signature << endl;
	glDeleteProgram(program);
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		cout << "ERROR DELETING PROGRAM" << endl;
	}
}

void ShaderProgram::loadShaderProgram()
{
	std::string shaderCodeString;
	std::ifstream shaderCodeStream(filePath, std::ios::in);
	if (shaderCodeStream.is_open()) {
		std::string Line = "";
		while (getline(shaderCodeStream, Line))
			shaderCodeString += "\n" + Line;
		shaderCodeStream.close();
		programString = shaderCodeString;
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", filePath);
	}
}

void ShaderProgram::bindShaderProgram(void)
{
	cout << "BINDING " << signature << " SHADER..." << endl;

	const char* pStringPointer = programString.c_str();

	program = glCreateShaderProgramv(shader, 1, &pStringPointer);

	// Check shader program
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &Result);

	cout << Result << " ACTIVE UNIFORMS" << endl;

	glGetProgramiv(program, GL_ATTACHED_SHADERS, &Result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> errorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(program, InfoLogLength, NULL, &errorMessage[0]);
		printf("%s\n", &errorMessage[0]);
	}
	else
	{
		compiledPrograms.push_back(this);
	}

	cout << endl;
}

void ShaderProgram::attachToPipeline(ShaderProgramPipeline* pipeline)
{
	pipeline->attachProgram(this);
}

GLint ShaderProgram::getLocationBySignature(string s)
{
	for (int i = 0; i < uniformIDs.size(); i++)
	{
		if (get<0>(uniformIDs[i]) == s)
		{
			return get<1>(uniformIDs[i]);
		}
	}

	return 0;
}

VertexShaderProgram::VertexShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature) : 
	ShaderProgram(filePath, uIDs, signature, GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT)
{
}

FragmentShaderProgram::FragmentShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature) :
	ShaderProgram(filePath, uIDs, signature, GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT)
{
}

GeometryShaderProgram::GeometryShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature) :
	ShaderProgram(filePath, uIDs, signature, GL_GEOMETRY_SHADER, GL_GEOMETRY_SHADER_BIT)
{

}