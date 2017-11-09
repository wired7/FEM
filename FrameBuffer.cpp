#include "FrameBuffer.h"
#include <iostream>

DecoratedFrameBuffer::DecoratedFrameBuffer(int width, int height, string signature) :
	Decorator<DecoratedFrameBuffer>(nullptr, signature), width(width), height(height)
{

}

DecoratedFrameBuffer::DecoratedFrameBuffer(DecoratedFrameBuffer* child, int width, int height, string signature) :
	Decorator<DecoratedFrameBuffer>(child, signature), width(width), height(height)
{
	FBO = child->FBO;
	attachmentNumber = child->attachmentNumber + 1;
}

void DecoratedFrameBuffer::bindFBO()
{
	if (FBO == 0)
	{
		glGenFramebuffers(1, &FBO);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void DecoratedFrameBuffer::bindTexture()
{

}

void DecoratedFrameBuffer::bindRBO()
{
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}

void DecoratedFrameBuffer::drawBuffers(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	vector<GLenum> buff;
	DecoratedFrameBuffer* currentBuffer = this;
	do {
		buff.insert(buff.begin(), GL_COLOR_ATTACHMENT0 + currentBuffer->attachmentNumber);
		currentBuffer = currentBuffer->child;
	} while (currentBuffer != nullptr);

	glDrawBuffers(buff.size(), &(buff[0]));
}

int DecoratedFrameBuffer::bindTexturesForPass(int textureOffset)
{
	vector<int> indices;
	DecoratedFrameBuffer* currentBuffer = this;
	int count = textureOffset;
	for (; currentBuffer != nullptr; count++)
	{
		currentBuffer = currentBuffer->child;
	}

	currentBuffer = this;
	for (int i = count - 1; i >= textureOffset; i--)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, currentBuffer->texture);
		currentBuffer = currentBuffer->child;
	}

	return count;
}

string DecoratedFrameBuffer::printOwnProperties(void)
{
	return to_string(attachmentNumber) + "\n";
}

DefaultFrameBuffer::DefaultFrameBuffer()
{
	FBO = 0;
	signature = "DEFAULT";
}

void DefaultFrameBuffer::bindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void DefaultFrameBuffer::bindTexture()
{

}

void DefaultFrameBuffer::bindRBO()
{

}

int DefaultFrameBuffer::bindTexturesForPass(int textureOffset)
{
	return 0;
}

ImageFrameBuffer::ImageFrameBuffer(int width, int height, string signature) : DecoratedFrameBuffer(width, height, signature)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ImageFrameBuffer::ImageFrameBuffer(DecoratedFrameBuffer* child, int width, int height, string signature) : DecoratedFrameBuffer(child, width, height, signature)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ImageFrameBuffer::bindTexture()
{
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, GL_TEXTURE_2D, texture, 0);
}

PickingBuffer::PickingBuffer(DecoratedFrameBuffer* child, int width, int height, string signature) : DecoratedFrameBuffer(child, width, height, signature)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PickingBuffer::PickingBuffer(int width, int height, string signature) : DecoratedFrameBuffer(width, height, signature)
{
	bindFBO();
	bindTexture();
	bindRBO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PickingBuffer::bindTexture()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto error = glGetError();

	if (error != GL_NO_ERROR)
		std::cout << error << std::endl;

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, GL_TEXTURE_2D, texture, 0);
}

GLuint* PickingBuffer::getValues(int x, int y)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentNumber);
	int length = 4 * width * height;

	GLuint* data = new GLuint[4];

	glReadPixels(x, y, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, data);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return data;
}