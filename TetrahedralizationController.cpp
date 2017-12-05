#include "TetrahedralizationController.h"
#include "SurfaceViewController.h"
#include "FPSCameraControls.h"
#include <thread>

#define REFRESH_RATE 0.1
bool TetrahedralizationController::stopUpdate = false;
bool TetrahedralizationController::isWorking = false;
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
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
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
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		stopUpdate = true;
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
		if (!isWorking) {
			isWorking = true;
			if (!controller->context->tetrahedralizationReady)
			{

				thread t([&] {
					double startTime = glfwGetTime();

					std::cout << "Triangulating... " << std::endl;

					for (int i = 0; i < controller->numberOfIterations; i++)
					{
						if (controller->context->addNextTetra(true) || stopUpdate) {
							stopUpdate = false;
							isWorking = false;
							break;
						}
						if (glfwGetTime() - startTime > REFRESH_RATE) {
							controller->context->tetrahedralizationReady = true;

							startTime = glfwGetTime();
						}
					}
					std::cout << "done" << std::endl;

					controller->context->tetrahedralizationReady = true;
					isWorking = false;
				});
				t.detach();
			}
		}

	}

	if (key == GLFW_KEY_U && action == GLFW_PRESS)
	{


		if (!isWorking) {
			isWorking = true;
			if (!controller->context->tetrahedralizationReady)
			{

				thread t([&] {
					double startTime = glfwGetTime();

					std::cout << "Triangulating... " << std::endl;

					for (int i = 0; i < controller->numberOfIterations; i++)
					{

						if (controller->context->addNextTetra(false) || stopUpdate) {
							stopUpdate = false;
							isWorking = false;
							break;
						}
						if (glfwGetTime() - startTime > REFRESH_RATE) {
							controller->context->tetrahedralizationReady = true;
							startTime = glfwGetTime();

						}
					}
					std::cout << "done " << std::endl;

					controller->context->tetrahedralizationReady = true;
					isWorking = false;
				});
				t.detach();
			}
		}

	}


	if (key == GLFW_KEY_PAGE_UP && action != GLFW_RELEASE)
	{
		if (!controller->context->tetrahedralizationReady) {
			controller->numberOfIterations *= 2;
		}
		cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;

	}
	if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
	{

		if (!controller->context->tetrahedralizationReady) {
			controller->numberOfIterations ++;
		}			
		cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;
	}

	if (key == GLFW_KEY_PAGE_DOWN && action != GLFW_RELEASE)
	{
		if (!controller->context->tetrahedralizationReady) {

			controller->numberOfIterations /= 2;
			if (controller->numberOfIterations < 1)
				controller->numberOfIterations;
		}
		cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;
	}

	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
	{
		if (!controller->context->tetrahedralizationReady) {

			controller->numberOfIterations--;
			if (controller->numberOfIterations < 1)
				controller->numberOfIterations;
		}
			cout << "NUMBER OF ITERATIONS PER CALL: " << controller->numberOfIterations << endl;
	}

	/*
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		if (controller->context->nextContext == nullptr)
		{
			auto geo = controller->context->geometries[0];
			auto cam = controller->context->cameras[0];

			controller->context->nextContext = new ClusteringStageContext(geo, cam);
			((ClusteringStageContext*)controller->context->nextContext)->prevContext = controller->context;
		}

		((ClusteringStageContext*)controller->context->nextContext)->geometries[0] = controller->context->geometries[0];
		((GeometryPass*)((ClusteringStageContext*)controller->context->nextContext)->passRootNode)->clearRenderableObjects(0);
		((GeometryPass*)((ClusteringStageContext*)controller->context->nextContext)->passRootNode)->addRenderableObjects(controller->context->geometries[0], 0);
		((ClusteringStageContext*)controller->context->nextContext)->setAsActiveContext();
	}
	*/
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			((PointSamplingContext*)controller->context->prevContext)->setAsActiveContext();
	}

	FPSCameraControls::cameraKeyboardControls<TetrahedralizationContext>(controller->context, key, action);
	
	controller->context->dirty = true;
	
}

void TetrahedralizationController::sC(GLFWwindow* window, double xOffset, double yOffset)
{

	//SurfaceViewController::cameraMovement(cam, xOffset, yOffset);

	//controller->context->dirty = true;
}

void TetrahedralizationController::mC(GLFWwindow* window, int button, int action, int mods)
{
}

void TetrahedralizationController::mPC(GLFWwindow* window, double xpos, double ypos)
{
	FPSCameraControls::mousePositionControls<TetrahedralizationController>(controller, xpos, ypos);
	
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}


