#pragma once
#include <glew.h>
#include <glm.hpp>
#include <iostream>
#include "Decorator.h"

class DecoratedFrameBuffer : public Decorator<DecoratedFrameBuffer>
{
protected:
	virtual void bindFBO(void);
	virtual void bindTexture(void) = 0;
	virtual void bindRBO(void);
public:
	GLuint FBO = 0;
	GLuint texture;
	GLuint RBO;
	int attachmentNumber = 0;
	int width;
	int height;
	DecoratedFrameBuffer() {};
	DecoratedFrameBuffer(int width, int height, string signature);
	DecoratedFrameBuffer(DecoratedFrameBuffer* child, int width, int height, string signature);
	~DecoratedFrameBuffer() {};

	void drawBuffers(void);
	void drawBuffer(string signature);
	void drawBuffers(vector<string> signatures);

	int bindTexturesForPass(int textureOffset = 0);
	void bindTexturesForPass(string signature);
	void bindTexturesForPass(vector<string> signatures);

	DecoratedFrameBuffer* make(void) { return NULL; };
	string printOwnProperties(void);
};

// TODO : make singleton pattern
class DefaultFrameBuffer : public DecoratedFrameBuffer
{
protected:
	void bindFBO(void);
	void bindTexture(void);
	void bindRBO(void);
	int bindTexturesForPass(int textureOffset = 0);
public:
	DefaultFrameBuffer();
	~DefaultFrameBuffer() {};
};

class ImageFrameBuffer : public DecoratedFrameBuffer
{
protected:
	virtual void bindTexture(void);
public:
	ImageFrameBuffer(int width, int height, string signature);
	ImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, string signature);
	~ImageFrameBuffer() {};
};

class PickingBuffer : public DecoratedFrameBuffer
{
protected:
	virtual void bindTexture(void);
public:
	PickingBuffer(int width, int height, string signature);
	PickingBuffer(DecoratedFrameBuffer* child, int width, int height, string signature);
	~PickingBuffer() {};

	GLuint* getValues(int x, int y);
};