#include "Controller.h"

using namespace std;

SurfaceViewController::SurfaceViewController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}

SurfaceViewController::~SurfaceViewController()
{
}

void SurfaceViewController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_N && action == GLFW_RELEASE)
	{
		auto n = new PointSamplingContext(controller->context->geometries[0], controller->context->cameras[0]);
		n->setAsActiveContext();
	}
}

void SurfaceViewController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	SphericalCamera* cam = controller->context->cameras[0];
	cameraMovement(cam, xOffset, yOffset);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);

	controller->context->dirty = true;
}

void SurfaceViewController::mC(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{

	}
	if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void SurfaceViewController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void SurfaceViewController::wRC(GLFWwindow*, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}

void SurfaceViewController::cameraMovement(SphericalCamera* cam, double xOffset, double yOffset)
{
	cam->camTheta -= 0.1 * (GLfloat)xOffset;
	cam->translate(5.0f * vec2(yOffset, yOffset));
	cam->update();
}

void SurfaceViewController::getPickingID(GeometryPass* gP, double xpos, double ypos)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);
	auto picking = (PickingBuffer*)gP->frameBuffer->signatureLookup("PICKING");
	auto data = picking->getValues(xpos, height - ypos);
	gP->setupOnHover(data[0]);

//	cout << data[0] << endl;

	delete[] data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointSamplingController::PointSamplingController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}

PointSamplingController::~PointSamplingController()
{
}

void PointSamplingController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void PointSamplingController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	SphericalCamera* cam = controller->context->cameras[0];
	SurfaceViewController::cameraMovement(cam, xOffset, yOffset);
	controller->context->dirty = true;
}

void PointSamplingController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void PointSamplingController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void PointSamplingController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}