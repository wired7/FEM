#include "ShaderProgramPipeline.h"
#include <iostream>
#include <string>

using namespace std;

vector<ShaderProgramPipeline*> ShaderProgramPipeline::pipelines;

ShaderProgramPipeline* ShaderProgramPipeline::getPipeline(string s)
{
	for (int i = 0; i < pipelines.size(); i++)
	{
		if (pipelines[i]->signature == s)
		{
			return pipelines[i];
		}
	}

	auto p = new ShaderProgramPipeline(s);

	for (int i = 0; i < pipelines.size(); i++)
	{
		if (pipelines[i]->signature == s)
		{
			return pipelines[i];
		}
	}

	delete p;
	return nullptr;
}

ShaderProgramPipeline::ShaderProgramPipeline(string s) : signature(s)
{
	glGetError();

	cout << "CREATING PROGRAM PIPELINE..." << endl;
	glGenProgramPipelines(1, &pipeline);
	glBindProgramPipeline(pipeline);
	
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		cout << "ERROR CREATING PROGRAM PIPELINE!" << endl;
	}
	else
	{
		pipelines.push_back(this);
	}

	cout << endl;
}

ShaderProgramPipeline::~ShaderProgramPipeline()
{
	glGetError();
	cout << "DELETING SHADER PROGRAM " << signature << endl;
	glDeleteProgramPipelines(1, &pipeline);
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		cout << "ERROR DELETING PROGRAM" << endl;
	}
}

void ShaderProgramPipeline::attachProgram(ShaderProgram* program)
{
	glGetError();

	cout << "ATTACHING " << program->signature << " TO PIPELINE " << pipeline << endl;

	glUseProgramStages(pipeline, program->shaderBit, program->program);
	
	// Check program pipeline
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetProgramPipelineiv(pipeline, program->shader, &Result);
	
	glGetProgramiv(program->program, GL_ACTIVE_UNIFORMS, &Result);

	cout << (int)Result << " ACTIVE UNIFORMS" << endl;

	GLenum error = glGetError();

	if (error == GL_NO_ERROR && Result == program->program)
	{
		cout << "SHADER ATTACHED SUCCESSFULY!" << endl;
	}
	else if (error != GL_NO_ERROR)
	{
		cout << "AN ERROR HAS OCCURRED" << endl;
	}

	cout << endl;

	glGetError();


	attachedPrograms.push_back(program);
}

void ShaderProgramPipeline::use(void)
{
	glBindProgramPipeline(pipeline);
}

ShaderProgram* ShaderProgramPipeline::getProgramBySignature(string signature)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		if (attachedPrograms[i]->signature == signature)
		{
			return attachedPrograms[i];
		}
	}

	return nullptr;
}

ShaderProgram* ShaderProgramPipeline::getProgramByEnum(GLenum e)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		if (attachedPrograms[i]->shader == e)
		{
			return attachedPrograms[i];
		}
	}

	return nullptr;
}

GLint ShaderProgramPipeline::getUniformByID(string signature)
{
	for (int i = 0; i < attachedPrograms.size(); i++)
	{
		for (int j = 0; j < attachedPrograms[i]->uniformIDs.size(); j++)
		{
			if (get<0>(attachedPrograms[i]->uniformIDs[j]) == signature)
			{
				return get<1>(attachedPrograms[i]->uniformIDs[j]);
			}
		}
	}

	return -1;
}