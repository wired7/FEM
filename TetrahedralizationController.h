#pragma once
#include "TetrahedralizationContext.h"

class TetrahedralizationContext;

class TetrahedralizationController : public Controller<TetrahedralizationController, TetrahedralizationContext>
{
public:
	bool surfaceRendering = false;
	TetrahedralizationController();
	~TetrahedralizationController();
	static void kC(GLFWwindow*, int, int, int, int);
	static void sC(GLFWwindow*, double, double);
	static void mC(GLFWwindow*, int, int, int);
	static void mPC(GLFWwindow*, double, double);
	static void wRC(GLFWwindow*, int, int);
	static void moveCamera(FPSCamera* cam, float xOffset, float yOffset);
	static void moveCamera(FPSCamera* cam, vec3 dir);
	
	static vec3 velocity;
	static bool firstMouse;
	static float lastX, lastY;

};