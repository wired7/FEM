#pragma once
#include "ClusteringStageController.h"
#include "SurfaceViewController.h"
#include "FPSCameraControls.h"

ClusteringStageController::ClusteringStageController()
{
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}


ClusteringStageController::~ClusteringStageController()
{
}

void ClusteringStageController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		((TetrahedralizationContext*)controller->context->prevContext)->setAsActiveContext();
	}

	FPSCameraControls::cameraKeyboardControls<ClusteringStageContext>(controller->context, key, action);

	controller->context->dirty = true;
}

void ClusteringStageController::sC(GLFWwindow* window, double xOffset, double yOffset)
{

}

void ClusteringStageController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void ClusteringStageController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls<ClusteringStageController>(controller, xpos, ypos);

	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void ClusteringStageController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}