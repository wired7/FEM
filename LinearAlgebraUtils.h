#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>

using namespace glm;

static class LinearAlgebraUtils
{
public:
	static mat4 getLineTransformFrom2Points(vec3 point1, vec3 point2);
};

