#include "Camera.h"

Camera* Camera::activeCamera = NULL;

Camera::Camera(GLFWwindow* window, vec2 relativePosition, vec2 relativeDimensions, vec3 pos, vec3 lookAt, vec3 up, mat4 Projection)
{
	this->window = window;
	this->relativePosition = relativePosition;
	this->relativeDimensions = relativeDimensions;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	mat4 ratioMatrix(1);
	ratioMatrix[1][1] *= relativeDimensions.x;
	Projection = Projection * ratioMatrix;
	OrthoProjection = glm::ortho(0.0f, (float)1200, 0.0f, (float)800);
	this->Projection = Projection;

	View = glm::lookAt(pos, lookAt, up);
	camPosVector = pos;
	lookAtVector = lookAt;
	upVector = up;
}

void Camera::setViewport()
{
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	glViewport(relativePosition.x * screenWidth, relativePosition.y * screenHeight, relativeDimensions.x * screenWidth, relativeDimensions.y * screenHeight);
}

void Camera::setCamera(Camera* cam)
{
	activeCamera = cam;
	activeCamera->setViewport();
}

int Camera::getScreenWidth()
{
	int width;
	int height;
	glfwGetWindowSize(window, &width, &height);
	return width;
}

int Camera::getScreenHeight()
{
	int width;
	int height;
	glfwGetWindowSize(window, &width, &height);
	return height;
}

GLFWwindow* Camera::getWindow()
{
	return window;
}

SphericalCamera::SphericalCamera(GLFWwindow* window, vec2 relativePosition, vec2 relativeDimensions, vec3 pos, vec3 lookAt, vec3 up, mat4 Projection) :
	Camera(window, relativePosition, relativeDimensions, pos, lookAt, up, Projection)
{
	distance = length(camPosVector - lookAtVector);
	maxCamPhi = 0.7;

	camTheta = atan2(lookAt.z - pos.z, lookAt.x - pos.x);

	update();
}

void SphericalCamera::update()
{
	if (camPhi > maxCamPhi)
		camPhi = maxCamPhi;
	else if (camPhi < maxCamPhi * -1)
		camPhi = maxCamPhi * -1;

	lookAtVector = camPosVector + vec3(cos(camTheta) * cos(camPhi), sin(camPhi), sin(camTheta) * cos(camPhi));

	View = lookAt(camPosVector, lookAtVector, upVector);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	Projection = perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
}

void SphericalCamera::translate(vec2 offset)
{
	vec3 diff(cos(camTheta), 0, sin(camTheta));
	camPosVector += diff * vec3(offset.x, 0, offset.y);
}