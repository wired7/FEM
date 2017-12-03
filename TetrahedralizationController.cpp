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
	if (key == GLFW_KEY_I && action == GLFW_RELEASE)
	{
		((TetrahedralizationContext*)controller->context)->addNextTetra();
		((TetrahedralizationContext*)controller->context)->fillUpGaps();
	}


	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) 
	{
		controller->context->cameras[0]->ToggleFpsMode();
	}

	if (key == GLFW_KEY_W )
	{
	//	moveCamera(controller->context->cameras[0], );
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
	std::cout << "new pos" << xpos << ", " << ypos << std::endl;
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


