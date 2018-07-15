#pragma once
#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include "HalfSimplices.h"
#include "FPSCameraControls.h"
#include "VoronoiDiagramUtils.h"
#include "LinearAlgebraUtils.h"
#include "DiscreteGeometryUtils.h"
#include <algorithm>    // std::sort
#include <omp.h>
#include <map>

using namespace Geometry;

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface,
													 Graphics::DecoratedGraphicsObject* points,
													 vector<vec3>& _points,
													 FPSCamera* cam,
													 ReferenceManager* refMan) : positions(_points), refMan(refMan)
{
	cameras.push_back(cam);
	geometries["SURFACE"] = surface;
	geometries["POINTS"] = points;
	setupGeometries();
	setupPasses({ "A", "EdgeA", "C", "D" }, { "B" });

	thread t([&]
	{
		auto tetrahedraPoints = VoronoiDiagramUtils::calculateDelaunayTetrahedra(positions);

		for (auto& tetra : tetrahedraPoints)
		{
			vector<GLuint> volumeIndices;

			for (int i = 0; i < tetra.length() - 2; ++i)
			{
				for (int j = i + 1; j < tetra.length() - 1; ++j)
				{
					for (int k = j + 1; k < tetra.length(); ++k)
					{
						int otherIndex;

						for (int l = 0; l < tetra.length(); ++l)
						{
							if (l != i && l != j && l != k)
							{
								otherIndex = l;
								break;
							}
						}

						vec3 p = positions[tetra[j]] - positions[tetra[i]];
						vec3 q = positions[tetra[k]] - positions[tetra[j]];

						if (glm::dot(glm::cross(p, q), positions[otherIndex] - positions[i]) > 0)
						{
							volumeIndices.push_back(tetra[k]);
							volumeIndices.push_back(tetra[j]);
							volumeIndices.push_back(tetra[i]);
						}
						else
						{
							volumeIndices.push_back(tetra[i]);
							volumeIndices.push_back(tetra[j]);
							volumeIndices.push_back(tetra[k]);
						}
					}
				}
			}
		}

		glm::vec3 centroid = (vec3(-1, 0, 0) + vec3(1, 0, 0) + vec3(0, 1, 0) + vec3(0, 0, 1)) / 4.0f;
		vec4 v1(-1, 0, 0, 1);
		vec4 v2(1, 0, 0, 1);
		vec4 v3(0, 1, 0, 1);
		vec4 v4(0, 0, 1, 1);

		auto triangles = DiscreteGeometryUtils::getTrianglesFromMesh(geometries["SURFACE"]);

//#pragma omp parallel for schedule(dynamic, 50)
		for (int tIndex = 0; tIndex < tetrahedraPoints.size(); ++tIndex)
		{
			auto tetraSet = tetrahedraPoints[tIndex];

			bool isInsideVolume = true;
			for (int i = 0; isInsideVolume && i < 2; ++i)
			{
				for (int j = i + 1; isInsideVolume && j < 3; ++j)
				{
					for (int k = j + 1; isInsideVolume && k < 4; ++k)
					{
						ImplicitGeo::Triangle triangle(positions[tetraSet[i]], positions[tetraSet[j]], positions[tetraSet[k]]);

						for (int triIndex = 0; triIndex < triangles.size(); ++triIndex)
						{
							if (triangles[triIndex].intersects(triangle))
							{
								isInsideVolume = false;
								break;
							}
						}
					}
				}
			}

			if (isInsideVolume)
			{
				auto transform = LinearAlgebraUtils::getTransformFrom4Points(
					positions[tetraSet[0]],
					positions[tetraSet[1]],
					positions[tetraSet[2]],
					positions[tetraSet[3]]) *
					translate(mat4(1.0f), centroid) *
					scale(mat4(1.0f), vec3(1.0f)) *
					translate(mat4(1.0f), -centroid);
//#pragma omp critical
				{
					tetraTransforms.push_back(transform);
				}
			}
		}

		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{
	makeQuad();
	dirty = true;
}

void TetrahedralizationContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["SURFACE"], "A");
	gP->addRenderableObjects(geometries["POINTS"], "C");

	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "B");
}

void TetrahedralizationContext::updateGeometries()
{
	auto meshObject = new Tetrahedron();
	auto instancedTetra = new MatrixInstancedMeshObject<mat4, float>(meshObject, tetraTransforms, "TETRAINSTANCES", 1);
	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, instancedTetra, tetraTransforms.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < tetraTransforms.size(); ++i)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	vector<mat4> parentTransforms;

	parentTransforms.push_back(scale(mat4(1.0f), vec3(1.0f)));

	geometries["VOLUMES"] = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM", tetraTransforms.size());

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["VOLUMES"], "D");
	dirty = true;
}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		controller->context->updateGeometries();
		tetrahedralizationReady = false;
	}

	if (length(cameras[0]->velocity) > 0)
	{
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}
}