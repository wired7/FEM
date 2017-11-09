#pragma once
#include <iostream>
#include <glew.h>
#include <glfw3.h>

using namespace std;

class WindowContext
{
private:
	WindowContext* context;
public:
	static GLFWwindow* window;
	WindowContext(int width, int height, const char* title);
	~WindowContext();
};

