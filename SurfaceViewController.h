#pragma once
#include "Controller.h"
#include "SurfaceViewContext.h"

class SurfaceViewContext;

class SurfaceViewController : public Controller<SurfaceViewController, SurfaceViewContext>
{
public:
	bool pointRendering = false;
	bool edgeRendering = false;
	bool surfaceRendering = false;
	SurfaceViewController();
	~SurfaceViewController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
	static void cameraMovement(SphericalCamera* cam, double xOffset, double yOffset);
	static void cameraMovement(SphericalCamera* cam, glm::vec3 direction);

	static void getPickingID(GeometryPass* gP, double xpos, double ypos);
};
