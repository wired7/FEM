#pragma once
#include <vector>
#include <glew.h>
#include "ShaderProgram.h"

using namespace std;

class ShaderProgram;

class ShaderProgramPipeline
{
private:
	ShaderProgramPipeline(string s);
	~ShaderProgramPipeline();
public:
	static vector<ShaderProgramPipeline*> pipelines;
	static ShaderProgramPipeline* getPipeline(string s);
	string signature;
	GLuint pipeline;
	vector<ShaderProgram*> attachedPrograms;
	void attachProgram(ShaderProgram* program);
	void use(void);
	ShaderProgram* getProgramBySignature(string s);
	ShaderProgram* getProgramByEnum(GLenum e);
	GLint getUniformByID(string s);
};

