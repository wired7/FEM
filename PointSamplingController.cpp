#pragma once
#include "PointSamplingController.h"
#include "SurfaceViewController.h"
#include "TetrahedralizationContext.h"

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
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		auto geo0 = controller->context->geometries[0];
		auto geo2 = controller->context->geometries[2];
		auto pass = (GeometryPass*)controller->context->passRootNode;

		pass->clearRenderableObjects(0);
		if (controller->surfaceRendering)
		{
			pass->addRenderableObjects(geo0, 0);
		}

		controller->surfaceRendering ^= true;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		if (controller->context->readyToAdvance)
		{
			if (controller->context->nextContext == nullptr)
			{
				auto geo = controller->context->geometries[0];
				auto &points = controller->context->points;
				auto cam = controller->context->cameras[0];

				controller->context->nextContext = new TetrahedralizationContext(geo, points, cam);
				((TetrahedralizationContext*)controller->context->nextContext)->prevContext = controller->context;
			}

			((TetrahedralizationContext*)controller->context->nextContext)->setAsActiveContext();
		}
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (controller->context->readyToAdvance)
		{
			((SurfaceViewContext*)controller->context->prevContext)->setAsActiveContext();
		}
	}

	controller->context->dirty = true;
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
