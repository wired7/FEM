#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>
#include <Eigen/QR>

static class LinearAlgebraUtils
{
public:
	static void init();
	static glm::mat4 getTransformFrom2Points(glm::vec3 point1, glm::vec3 point2);
	static glm::mat4 getTransformFrom3Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	static glm::mat4 getTransformFrom4Points(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

	static Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> originalEdgePseudo;
	static Eigen::Matrix4d originalTrianglePseudo;
	static Eigen::Matrix4d originalTetraPseudo;
};

