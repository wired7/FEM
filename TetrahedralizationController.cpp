#include "TetrahedralizationController.h"
#include "SurfaceViewController.h"

TetrahedralizationController::TetrahedralizationController() {
	key_callback = kC;
	scroll_callback = sC;
	mouse_callback = mC;
	mousePos_callback = mPC;
	windowResize_callback = wRC;
}
TetrahedralizationController::~TetrahedralizationController() {

}

void TetrahedralizationController::kC(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_I && action == GLFW_RELEASE)
	{
		((TetrahedralizationContext*)controller->context)->addNextFacet();
	}


	controller->context->dirty = true;
}

void TetrahedralizationController::sC(GLFWwindow* window, double xOffset, double yOffset)
{
	SphericalCamera* cam = controller->context->cameras[0];
	SurfaceViewController::cameraMovement(cam, xOffset, yOffset);
	controller->context->dirty = true;
}

void TetrahedralizationController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void TetrahedralizationController::mPC(GLFWwindow* window, double xpos, double ypos)
{
//	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}
