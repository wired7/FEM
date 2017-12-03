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


	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		controller->surfaceRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects(0);
		if (controller->volumeRendering)
		{
			auto geo0 = controller->context->geometries[0];
			pass->addRenderableObjects(geo0, 0);
		}

		if (controller->surfaceRendering)
		{
			auto geo1 = controller->context->geometries[4];
			pass->addRenderableObjects(geo1, 0);
		}
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		controller->facetRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects(3);
		if (controller->facetRendering)
		{
			auto geo0 = controller->context->geometries[3];
			pass->addRenderableObjects(geo0, 3);
		}
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		controller->edgeRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects(1);
		if (controller->edgeRendering)
		{
			auto geo0 = controller->context->geometries[1];
			pass->addRenderableObjects(geo0, 1);
		}
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		controller->pointRendering ^= true;

		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects(2);
		if (controller->pointRendering)
		{
			auto geo0 = controller->context->geometries[2];
			pass->addRenderableObjects(geo0, 2);
		}
	}

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		for (int i = 0; i < controller->numberOfIterations; i++)
		{
			controller->context->addNextFacet();
		}

		controller->context->updateGeometries();
	}

	if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
	{
		if (controller->numberOfIterations < 100)
		{
			controller->numberOfIterations++;
			cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;
		}
	}

	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
	{
		if (controller->numberOfIterations > 1)
		{
			controller->numberOfIterations--;
			cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;
		}
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
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}
