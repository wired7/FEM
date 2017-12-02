#include "LinearAlgebraUtils.h"

mat4 LinearAlgebraUtils::getLineTransformFrom2Points(vec3 point1, vec3 point2)
{
	vec3 vectorDir = normalize(point2 - point1);
	vec3 z;
	if (!(abs(vectorDir.x) == 1.0f && vectorDir.y == 0.0f && vectorDir.z == 0.0f))
	{
		z = -normalize(cross(vectorDir, vec3(1, 0, 0)));
	}
	else if (abs(vectorDir.x) == 1.0f)
	{
		z = vec3(0, 0, -1);
	}
	else
	{
		z = vec3(0, 0, 1);
	}

	float angle = acos(dot(normalize(point2 - point1), vec3(1, 0, 0)));

	return translate(mat4(1.0f), point1) * rotate(mat4(1.0f), angle, z);
}