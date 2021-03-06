#pragma once
#include "Controller.h"
#include "SurfaceViewContext.h"

class SurfaceViewContext;
class FPSCameraInterface;

class SurfaceViewController : public Controller<SurfaceViewController, SurfaceViewContext>
{
public:
	bool pointRendering = true;
	bool edgeRendering = true;
	bool surfaceRendering = true;
	bool facetRendering = true;

	bool firstMouse = false;
	float lastX = 0;
	float lastY = 0;

	SurfaceViewController();
	~SurfaceViewController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
	static void cameraMovement(SphericalCamera* cam, double xOffset, double yOffset);
	static void cameraMovement(SphericalCamera* cam, glm::vec3 direction);

	static unsigned int getPickingID(GeometryPass* gP, double xpos, double ypos);
};
