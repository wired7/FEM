#pragma once
#include <vector>
#include <functional>
#include "WindowContext.h"
#include "Context.h"

template<class T, class S> class Controller
{
protected:
	static T* controller;
	GLFWwindow* window;
	Controller();
	~Controller() {};
	void(*key_callback)(GLFWwindow*, int, int, int, int) = 0;
	void(*scroll_callback)(GLFWwindow*, double, double) = 0;
	void(*mouse_callback)(GLFWwindow*, int, int, int) = 0;
	void(*mousePos_callback)(GLFWwindow*, double, double) = 0;
	void(*windowResize_callback)(GLFWwindow*, int, int) = 0;
public:
	S* context;
	static T* getController();
	void setController();
	void setContext(S* context);
};

#pragma region ControllerTemplate
template<class T, class S> T* Controller<T, S>::controller = nullptr;

template<class T, class S> Controller<T, S>::Controller() : window(WindowContext::window)
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

class SurfaceViewContext;
class GeometryPass;

class SurfaceViewController : public Controller<SurfaceViewController, SurfaceViewContext>
{
public:
	SurfaceViewController();
	~SurfaceViewController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
	static void cameraMovement(SphericalCamera* cam, double xOffset, double yOffset);
	static void getPickingID(GeometryPass* gP, double xpos, double ypos);
};

class PointSamplingContext;

class PointSamplingController : public Controller<PointSamplingController, PointSamplingContext>
{
public:
	PointSamplingController();
	~PointSamplingController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
};