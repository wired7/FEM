#pragma once
#include <CL/cl.h>
#include <CL/opencl.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <glew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "glfw3.h"
#include "glfw3native.h"

using namespace std;

class CLKernel;
class OpenCLContext
{
public:
	cl_context context;
	vector<cl_platform_id> platformIDs;
	vector<cl_device_id> deviceIDs;
	vector<cl_device_type> deviceTypes;
	cl_int ret;
	vector<cl_command_queue> commandQueues;
	OpenCLContext(GLFWwindow* window = nullptr);
	~OpenCLContext();
	template<typename T> void platformInfo(cl_platform_id platform_id, cl_platform_info info);
	template<typename T> void deviceInfo(cl_device_id device_id, cl_device_info info);
};

template<typename T> void OpenCLContext::platformInfo(cl_platform_id platform_id, cl_platform_info info)
{
	size_t stringSize;
	clGetPlatformInfo(platform_id, info, NULL, NULL, &stringSize);
	T* output = new T[stringSize];
	clGetPlatformInfo(platform_id, info, stringSize * sizeof(T), output, NULL);
	cout << output << endl;
	delete[] output;
}

template<typename T> void OpenCLContext::deviceInfo(cl_device_id device_id, cl_device_info info)
{
	size_t stringSize;
	clGetDeviceInfo(device_id, info, NULL, NULL, &stringSize);
	T* output = new T[stringSize];
	clGetDeviceInfo(device_id, info, stringSize * sizeof(T), output, NULL);
	cout << output << endl;
	delete[] output;
}

class CLKernel
{
public:
	string kernelString;
	cl_kernel kernel;
	OpenCLContext* context;
	CLKernel(string filePath, string kernelName, OpenCLContext* context);
	~CLKernel();
	void readFile(string filePath);
	void execute(size_t global_item_size, size_t local_item_size);
};

template<class T> class CLBuffer
{
public:
	vector<T> bufferData;
	cl_mem bufferPointer;
	OpenCLContext* context;
	cl_int ret;
	int readWrite;
	CLBuffer() {};
	CLBuffer(OpenCLContext* context, vector<T>& data, int readOnly = CL_MEM_READ_ONLY);
	~CLBuffer();
	void bindBuffer();
	void updateBuffer();
	virtual void enableBuffer(CLKernel* kernel, int argumentIndex);
	void readBuffer();
};

template<class T> CLBuffer<T>::CLBuffer(OpenCLContext* context, vector<T>& data, int readWrite) :
	context(context), bufferData(data), readWrite(readWrite)
{
	
}

template<class T> CLBuffer<T>::~CLBuffer()
{

}

template<class T> void CLBuffer<T>::bindBuffer()
{
	bufferPointer = clCreateBuffer(context->context, readWrite, bufferData.size() * sizeof(T), NULL, &ret);
	updateBuffer();
}

template<class T> void CLBuffer<T>::updateBuffer()
{
	vector<T> v(bufferData);
	ret = clEnqueueWriteBuffer(context->commandQueues[0], bufferPointer, CL_TRUE, 0, v.size() * sizeof(T), &(v[0]), 0, NULL, NULL);
}

template<class T> void CLBuffer<T>::enableBuffer(CLKernel* kernel, int argumentIndex)
{
	ret = clSetKernelArg(kernel->kernel, argumentIndex, sizeof(cl_mem), (void *)&bufferPointer);
}

template<class T> void CLBuffer<T>::readBuffer()
{
	vector<T> v;
	v.resize(bufferData.size());

	ret = clEnqueueReadBuffer(context->commandQueues[0], bufferPointer, CL_FALSE, 0, v.size() * sizeof(T), &(v[0]), 0, NULL, NULL);
	bufferData = v;
}

template<class T> class CLGLBuffer : public CLBuffer<T>
{
public:
	CLGLBuffer(OpenCLContext* context, GLuint VBO, int readOnly = CL_MEM_READ_ONLY);
	~CLGLBuffer();
};

template<class T> CLGLBuffer<T>::CLGLBuffer(OpenCLContext* context, GLuint VBO, int readWrite)
{
	this->context = context;
	this->readWrite = readWrite;
	bufferPointer = clCreateFromGLBuffer(context->context, readWrite, VBO, &ret);

}

template<class T> CLGLBuffer<T>::~CLGLBuffer()
{

}