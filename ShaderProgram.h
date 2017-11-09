#pragma once
#include <tuple>
#include <vector>
#include "ShaderProgramPipeline.h"

using namespace std;

class ShaderProgramPipeline;

enum UniformType {ONEUI, MATRIX4FV, TEXTURE};

class ShaderProgram
{
protected:
	static ShaderProgram* getCompiledProgram(string filePath);
	static ShaderProgram* getCompiledProgramBySignature(string signature);
	ShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature, GLenum shader, GLenum shaderBit);
	~ShaderProgram();
public:
	static vector<ShaderProgram*> compiledPrograms;
	template<class T> static T* getShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature);
	template<class T> static T* getShaderProgram(string filePath);
	string filePath;
	string programString;
	GLuint program;
	GLenum shader;
	GLenum shaderBit;
	string signature;
	vector<tuple<string, GLint, UniformType>> uniformIDs;
	virtual void bindShaderProgram();
	virtual void loadShaderProgram();
	virtual void attachToPipeline(ShaderProgramPipeline* pipeline);
	GLint getLocationBySignature(string s);
};

template<class T> T* ShaderProgram::getShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature)
{
	T* prog = (T*)getCompiledProgram(filePath);

	if (prog == nullptr)
	{
		prog = new T(filePath, uIDs, signature);

		if (getCompiledProgram(filePath) == nullptr)
		{
			delete prog;
			prog = nullptr;
		}
	}

	return prog;
}

template<class T> T* ShaderProgram::getShaderProgram(string signature)
{
	return (T*)getCompiledProgramBySignature(signature);
}

class VertexShaderProgram : public ShaderProgram
{
public:
	VertexShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature);
	~VertexShaderProgram() {};
};

class FragmentShaderProgram : public ShaderProgram
{
public:
	FragmentShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature);
	~FragmentShaderProgram() {};
};

class GeometryShaderProgram : public ShaderProgram
{
public:
	GeometryShaderProgram(string filePath, vector<tuple<const GLchar*, UniformType>> uIDs, string signature);
	~GeometryShaderProgram() {};
};