#include "TetrahedralizationController.h"
#include "SurfaceViewController.h"



vec3 TetrahedralizationController::velocity = vec3(0);
bool TetrahedralizationController::firstMouse = false;
float TetrahedralizationController::lastX = 0;
float TetrahedralizationController::lastY = 0;

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
			controller->context->addNextTetra();
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

	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) 
	{
		controller->context->cameras[0]->ToggleFpsMode();
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
	{
		controller->context->cameras[0]->changeSpeed();
	}

	if (key == GLFW_KEY_W )
	{
		if(action == GLFW_PRESS)
			velocity += glm::vec3(0, 0, 1);
		if(action == GLFW_RELEASE)
			velocity -= glm::vec3(0, 0, 1);
	}

	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			velocity += glm::vec3(0, 0, -1);
		if (action == GLFW_RELEASE)
			velocity -= glm::vec3(0, 0, -1);

	}
	if (key == GLFW_KEY_A )
	{
		//moveCamera(controller->context->cameras[0], glm::vec3(-1, 0, 0) );
		if (action == GLFW_PRESS)
			velocity += glm::vec3(-1, 0, 0);
		if (action == GLFW_RELEASE)
			velocity -= glm::vec3(-1, 0, 0);

	}
	if (key == GLFW_KEY_D)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			velocity += glm::vec3(1, 0, 0);
		if (action == GLFW_RELEASE)
			velocity -= glm::vec3(1, 0, 0);
	}

	if (key == GLFW_KEY_SPACE)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			velocity += glm::vec3(0, 1, 0);
		if (action == GLFW_RELEASE)
			velocity -= glm::vec3(0, 1, 0);
	}

	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		//	moveCamera(controller->context->cameras[0], glm::vec3(1, 0, 0) );
		if (action == GLFW_PRESS)
			velocity += glm::vec3(0, -1, 0);
		if (action == GLFW_RELEASE)
			velocity -= glm::vec3(0, -1, 0);
	}
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
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	//camera->ProcessMouseMovement(xoffset, yoffset);

	moveCamera(controller->context->cameras[0], xoffset, yoffset);
	SurfaceViewController::getPickingID((GeometryPass*)controller->context->passRootNode, xpos, ypos);
	controller->context->dirty = true;
}

void TetrahedralizationController::wRC(GLFWwindow* window, int a, int b)
{
	controller->context->cameras[0]->update();
	controller->context->dirty = true;
}

void TetrahedralizationController::moveCamera(FPSCamera* cam, float xOffset, float yOffset) {
	if (cam->usingCamera){
		cam->processMouseMovement(xOffset, yOffset);
	}

}

void TetrahedralizationController::moveCamera(FPSCamera* cam, glm::vec3 dir) {
		cam->processKeyInput(dir);
		cam->update();
}


