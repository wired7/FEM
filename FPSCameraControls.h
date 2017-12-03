#pragma once
#include "SurfaceViewContext.h"
#include "PointSamplingContext.h"
#include "SurfaceViewContext.h"
#include "TetrahedralizationContext.h"
#include "ClusteringStageContext.h"

static class FPSCameraControls
{
public:
	template <class T> static void cameraKeyboardControls(T* context, int key, int action);
	template <class T> static void mousePositionControls(T* controller, double xpos, double ypos);
	static void moveCamera(FPSCamera* cam, float xOffset, float yOffset);
	static void moveCamera(FPSCamera* cam, vec3 dir);
};

template <class T> void FPSCameraControls::cameraKeyboardControls(T* context, int key, int action)
{
	// camera controls
	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE)
	{
		context->cameras[0]->ToggleFpsMode();
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
	{
		context->cameras[0]->changeSpeed();
	}

	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(0, 0, 1);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(0, 0, 1);
	}

	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(0, 0, -1);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(0, 0, -1);

	}
	if (key == GLFW_KEY_A)
	{
		//moveCamera(controller->context->cameras[0], glm::vec3(-1, 0, 0) );
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(-1, 0, 0);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(-1, 0, 0);

	}
	if (key == GLFW_KEY_D)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(1, 0, 0);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(1, 0, 0);
	}
	if (key == GLFW_KEY_SPACE)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(0, 1, 0);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(0, 1, 0);
	}
	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			context->cameras[0]->velocity += glm::vec3(0, -1, 0);
		if (action == GLFW_RELEASE)
			context->cameras[0]->velocity -= glm::vec3(0, -1, 0);
	}
}

template <class T> void FPSCameraControls::mousePositionControls(T* controller, double xpos, double ypos)
{
	if (controller->firstMouse)
	{
		controller->lastX = xpos;
		controller->lastY = ypos;
		controller->firstMouse = false;
	}

	float xoffset = xpos - controller->lastX;
	float yoffset = controller->lastY - ypos; // reversed since y-coordinates go from bottom to top

	controller->lastX = xpos;
	controller->lastY = ypos;

	//camera->ProcessMouseMovement(xoffset, yoffset);

	FPSCameraControls::moveCamera(controller->context->cameras[0], xoffset, yoffset);
}