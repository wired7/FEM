#pragma once
#include <vector>
#include <functional>
#include "WindowContext.h"
#include "Context.h"

class AbstractController
{
protected:
	GLFWwindow* window;
	void(*key_callback)(GLFWwindow*, int, int, int, int) = 0;
	void(*scroll_callback)(GLFWwindow*, double, double) = 0;
	void(*mouse_callback)(GLFWwindow*, int, int, int) = 0;
	void(*mousePos_callback)(GLFWwindow*, double, double) = 0;
	void(*windowResize_callback)(GLFWwindow*, int, int) = 0;
public:
	AbstractController() : window(WindowContext::window) {};
};

template<class T, class S> class Controller : public AbstractController
{
protected:
	static T* controller;
	Controller();
	~Controller() {};
public:
	S* context;
	static T* getController();
	void setContext(S* context);
	void setController();
};

#pragma region ControllerTemplate
template<class T, class S> T* Controller<T, S>::controller = nullptr;

template<class T, class S> Controller<T, S>::Controller() : AbstractController()
{
}

template<class T, class S> void Controller<T, S>::setController()
{
	controller = (T*)this;

	glfwSetKeyCallback(WindowContext::window, key_callback);
	glfwSetScrollCallback(WindowContext::window, scroll_callback);
	glfwSetMouseButtonCallback(WindowContext::window, mouse_callback);
	glfwSetCursorPosCallback(WindowContext::window, mousePos_callback);
	glfwSetWindowSizeCallback(WindowContext::window, windowResize_callback);
}

template<class T, class S> T* Controller<T, S>::getController()
{
	if (controller == nullptr)
	{
		controller = new T();
	}

	return controller;
}

template<class T, class S> void Controller<T, S>::setContext(S* context)
{
	this->context = context;
}
#pragma endregion