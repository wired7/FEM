#pragma once
#include "FPSCameraControls.h"

void FPSCameraControls::moveCamera(FPSCamera* cam, float xOffset, float yOffset) {
	if (cam->usingCamera) {
		cam->processMouseMovement(xOffset, yOffset);
	}
}

void FPSCameraControls::moveCamera(FPSCamera* cam, glm::vec3 dir) {
	cam->processKeyInput(dir);
	cam->update();
}