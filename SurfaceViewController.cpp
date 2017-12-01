#include "SurfaceViewController.h"
#include "PointSamplingContext.h"

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
		auto geo = controller->context->geometries[0];
		auto cam = controller->context->cameras[0];

		auto n = new PointSamplingContext(geo, cam);
		n->setAsActiveContext();
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		auto geo1 = controller->context->geometries[1];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->edgeRendering)
		{
			pass->addRenderableObjects(geo1, 1);
		}
		else
		{
			pass->clearRenderableObjects(1);
		}

		controller->edgeRendering ^= true;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		auto geo0 = controller->context->geometries[0];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->surfaceRendering)
		{
			pass->addRenderableObjects(geo0, 0);
		}
		else
		{
			pass->clearRenderableObjects(0);
		}

		controller->surfaceRendering ^= true;
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		auto geo0 = controller->context->geometries[2];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->pointRendering)
		{
			pass->addRenderableObjects(geo0, 2);
		}
		else
		{
			pass->clearRenderableObjects(2);
		}

		controller->pointRendering ^= true;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		auto geo0 = controller->context->geometries[3];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		if (controller->pointRendering)
		{
			pass->addRenderableObjects(geo0, 3);
		}
		else
		{
			pass->clearRenderableObjects(3);
		}

		controller->pointRendering ^= true;
	}

	if (key == GLFW_KEY_SPACE && action != GLFW_PRESS) {
		
		SphericalCamera* cam = controller->context->cameras[0];
		cameraMovement(cam,glm::vec3(0, 1, 0));

	}
	if (key == GLFW_KEY_LEFT_CONTROL && action != GLFW_PRESS) {

		SphericalCamera* cam = controller->context->cameras[0];
		cameraMovement(cam,glm::vec3(0, -1, 0));

	}


	controller->context->dirty = true;
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
void SurfaceViewController::cameraMovement(SphericalCamera* cam, glm::vec3 direction)
{
	cam->camPosVector += direction*0.3f;
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