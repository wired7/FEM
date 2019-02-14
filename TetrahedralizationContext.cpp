#pragma once
#include "TetrahedralizationContext.h"
#include "TetrahedralizationController.h"

#include "HalfSimplices.h"
#include "HalfSimplexGeometryUtils.h"
#include "FPSCameraControls.h"
#include "VoronoiDiagramUtils.h"
#include "LinearAlgebraUtils.h"
#include "DiscreteGeometryUtils.h"
#include <algorithm>    // std::sort
#include <omp.h>
#include <map>
#include <unordered_set>
#include <limits>

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
	setupPasses({ "A", "A2", "EdgeA", "C", "D" }, { "B" });

	std::vector<ImplicitGeo::Triangle> surfaceTriangles = DiscreteGeometryUtils::getTrianglesFromMesh(surface);

	thread t([=]
	{
		manifold = new Geometry::Manifold3<GLuint>();

		std::vector<glm::vec3> samplePoints;
		samplePoints.push_back(glm::vec3(-100000));
		samplePoints.push_back(glm::vec3(0, 100000, 0));
		samplePoints.push_back(glm::vec3(1, -1, -1) * 100000.0f);
		samplePoints.push_back(glm::vec3(0, 0, 100000));
		samplePoints.insert(samplePoints.end(), positions.begin(), positions.end());

		positions = samplePoints;

		std::vector<GLuint> tetraIndices = { 0, 1, 2, 3 };

		manifold->add3(std::make_pair(tetraIndices, nullptr));

		std::queue<GLuint> insertionIndices;

		for (GLuint i = 4; i < positions.size(); ++i)
		{
			insertionIndices.push(i);
		}

		std::map<GLuint, int> deferredIndices;

		while(!insertionIndices.empty())
		{
			GLuint i = insertionIndices.front();
			insertionIndices.pop();

			vec3 targetPoint = positions[i];
			auto currentSimplex = manifold->map3.begin()->second;
			Geometry::HalfSimplex<3, GLuint>* targetSimplex =
				HalfSimplexGeometryUtils::findSimplex<3, vec3>(currentSimplex, positions, targetPoint,
					[](Geometry::HalfSimplex<3, GLuint>* currentSimplex, std::vector<vec3>& positions, vec3 targetPoint) -> bool
			{
					std::vector<Geometry::TopologicalStruct*> vertexVec;
					std::unordered_set<Geometry::TopologicalStruct*> vertexSet;
					currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);
					std::vector<vec3> simplexVertices;

					for (const auto& vertex : vertexVec)
					{
						simplexVertices.push_back(positions[reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->halfSimplexData]);
					}

					auto sphere = VoronoiDiagramUtils::getCircumsphere(&(simplexVertices[0]));
					return length(targetPoint - sphere.center) < sphere.radius;
			});

			std::map<std::vector<GLuint>, std::vector<std::vector<GLuint>>> facetMap;
			std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> visitedSimplices;
			std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> simplicesToErase;
			std::queue<Geometry::HalfSimplex<3, GLuint>*> simplexQueue;
			simplexQueue.push(targetSimplex);
			bool failed = false;
			while (simplexQueue.size())
			{
				auto currentSimplex = simplexQueue.front();
				simplexQueue.pop();

				if (visitedSimplices.find(currentSimplex) != visitedSimplices.end())
				{
					continue;
				}

				visitedSimplices.insert(currentSimplex);

				std::vector<TopologicalStruct*> targetSimplexVerticesVec;
				std::unordered_set<TopologicalStruct*> targetSimplexVerticesSet;
				currentSimplex->getAllNthChildren(targetSimplexVerticesVec, targetSimplexVerticesSet, 0);
				std::vector<vec3> simplexPositions;
				for (const auto& targetSimplexVertex : targetSimplexVerticesVec)
				{
					simplexPositions.push_back(positions[reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(targetSimplexVertex)->halfSimplexData]);
				}

				auto sphere = VoronoiDiagramUtils::getCircumsphere(&(simplexPositions[0]));
				auto distance = length(positions[i] - sphere.center);

				if (distance > (sphere.radius - 0.00001) && distance < (sphere.radius + 0.00001))
				{
					failed = true;
					break;
				}

				if (distance > (sphere.radius + 0.00001))
				{
					continue;
				}

				simplicesToErase.insert(currentSimplex);

				std::vector<TopologicalStruct*> targetSimplexFacetsVec;
				std::unordered_set<TopologicalStruct*> targetSimplexFacetsSet;
				currentSimplex->getAllChildren(targetSimplexFacetsVec, targetSimplexFacetsSet);

				for (const auto& facet : targetSimplexFacetsVec)
				{
					auto halfFacet = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet);

					if (halfFacet->twin != nullptr)
					{
						simplexQueue.push(halfFacet->twin->belongsTo);
					}

					std::vector<TopologicalStruct*> facetVertexVec;
					std::unordered_set<TopologicalStruct*> facetVertexSet;
					facet->getAllNthChildren(facetVertexVec, facetVertexSet, 0);

					// obtain facet indices
					std::vector<GLuint> facetIndices;
					for (const auto& facetIndex : facetVertexVec)
					{
						facetIndices.push_back(reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetIndex)->halfSimplexData);
					}

					auto sortedFacetIndices = facetIndices;
					std::sort(sortedFacetIndices.begin(), sortedFacetIndices.end());

					facetMap[sortedFacetIndices].push_back(arrangeSimplexIndices<GLuint>(facetIndices));

					if (facetMap[sortedFacetIndices].size() > 1)
					{
						facetMap.erase(sortedFacetIndices);
					}
				}
			}

			if (failed)
			{
				if (deferredIndices[i] < 3)
				{
					insertionIndices.push(i);
				}

				deferredIndices[i]++;
				continue;
			}

			for (const auto& simplex : simplicesToErase)
			{
				manifold->erase3(simplex);
			}

			for (auto& facet : facetMap)
			{
				facet.second[0].push_back(i);
				auto newTetra = arrangeSimplexIndices<GLuint>(facet.second[0]);

				if (newTetra != facet.second[0])
				{
					std::reverse(newTetra.begin() + 1, newTetra.end());
				}

				std::vector<std::vector<GLuint>> tetraArrangements = { newTetra, { newTetra[0], newTetra[2], newTetra[3], newTetra[1] },
																		{ newTetra[0], newTetra[3], newTetra[1], newTetra[2] } };

				bool nonePassed = true;
				for(const auto& arrangement : tetraArrangements)
				{
					bool arrangementFailed = false;
					for (int j = 0; j < 4 && !arrangementFailed; ++j)
					{
						std::vector<GLuint> facetIndices;
						for (int k = 0; k < 3; ++k)
						{
							facetIndices.push_back(arrangement[(j + k) % 4]);
						}

						auto sortedIndices = facetIndices;

						std::sort(sortedIndices.begin(), sortedIndices.end());

						if (j % 2)
						{
							std::reverse(facetIndices.begin(), facetIndices.end());
						}

						facetIndices = arrangeSimplexIndices<GLuint>(facetIndices);

						auto twins = manifold->twinMap2[std::make_pair(sortedIndices, nullptr)];
						if (twins.size())
						{
							for (const auto& twin : twins)
							{
								if (twin.first.first == facetIndices)
								{
									arrangementFailed = true;
									break;
								}
							}
						}
					}

					if (!arrangementFailed)
					{
						manifold->add3(std::make_pair(arrangement, nullptr));
						nonePassed = false;
						break;
					}
				}
			}
		}

		std::vector<GLuint> backgroundSimplexVertices = { 0, 1, 2, 3};
		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> backgroundSimplexTetras;
		for (const auto& tetra : manifold->map3)
		{
			for (const auto& tetraVertex : tetra.first.first)
			{
				if (std::find(backgroundSimplexVertices.begin(), backgroundSimplexVertices.end(), tetraVertex) != backgroundSimplexVertices.end())
				{
					backgroundSimplexTetras.insert(tetra.second);
					break;
				}
			}
		}

		for (const auto& tetra : backgroundSimplexTetras)
		{
			manifold->erase3(tetra);
		}

		auto contourCriterion = [](Geometry::HalfSimplex<3, GLuint>* currentSimplex) -> bool
		{
			std::vector<Geometry::TopologicalStruct*> facetVec;
			std::unordered_set<Geometry::TopologicalStruct*> facetSet;
			currentSimplex->getAllChildren(facetVec, facetSet);

			for (const auto& facet : facetVec)
			{
				if (reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet)->twin == nullptr)
				{
					return true;
				}
			}

			return false;
		};

		auto initialSimplex = manifold->map3.begin()->second;
		Geometry::HalfSimplex<3, GLuint>* simplexOnContour =
			HalfSimplexGeometryUtils::findSimplex<3, vec3>(initialSimplex, positions, vec3(0, 10000, 0),
				[contourCriterion](Geometry::HalfSimplex<3, GLuint>* currentSimplex, std::vector<vec3>& positions, vec3 targetPoint) {			
					return contourCriterion(currentSimplex);
				});

		std::queue<Geometry::HalfSimplex<3, GLuint>*> simplexToExplore;
		simplexToExplore.push(simplexOnContour);
		
		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> contourSet;
		std::queue<Geometry::HalfSimplex<3, GLuint>*> contourQueue;

		for (const auto& mappedSimplex : manifold->map3)
		{
			auto currentSimplex = mappedSimplex.second;
			std::vector<TopologicalStruct*> simplexFacetsVec;
			std::unordered_set<TopologicalStruct*> simplexFacetsSet;
			currentSimplex->getAllChildren(simplexFacetsVec, simplexFacetsSet);
 			bool onContour = false;
			for (const auto& facet : simplexFacetsVec)
			{
				auto halfFacet = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet);

				if (halfFacet->twin != nullptr)
				{
					simplexToExplore.push(halfFacet->twin->belongsTo);
				}
				else
				{
					onContour = true;
				}
			}

			if (onContour)
			{
				if (contourSet.insert(currentSimplex).second)
				{
					contourQueue.push(currentSimplex);
				}
			}
		}

		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> visitedSet;
		while (!contourQueue.empty())
		{
			auto currentSimplex = contourQueue.front();
			contourQueue.pop();

			if (visitedSet.find(currentSimplex) != visitedSet.end())
			{
				continue;
			}

			visitedSet.insert(currentSimplex);

			std::vector<TopologicalStruct*> vertexVec;
			std::unordered_set<TopologicalStruct*> vertexSet;
			currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);

			std::vector<vec3> simplexVertices;

			for (const auto& vertex : vertexVec)
			{
				simplexVertices.push_back(positions[reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->halfSimplexData]);
			}
			
			bool toBeRemoved = false;

			vec3 centroid;
			for (const auto& vertex : simplexVertices)
			{
				centroid += vertex;
			}
			centroid /= 4.0f;

			if (!DiscreteGeometryUtils::isPointInsideMesh(centroid, surfaceTriangles))
			{
				toBeRemoved = true;
			}
			else
			{
				std::vector<vec3> halfwayVertices;
				for (const auto& vertex : simplexVertices)
				{
					auto halfwayVertex = (centroid + vertex) / 2.0f;
					if (!DiscreteGeometryUtils::isPointInsideMesh(halfwayVertex, surfaceTriangles))
					{
						toBeRemoved = true;
						break;
					}

					halfwayVertices.push_back(halfwayVertex);
				}

				if (!toBeRemoved)
				{
					for (int j = 0; j < 3 && !toBeRemoved; ++j)
					{
						for (int k = j; k < 4 && !toBeRemoved; ++k)
						{
							if (!DiscreteGeometryUtils::isPointInsideMesh((halfwayVertices[j] + halfwayVertices[k]) / 2.0f, surfaceTriangles))
							{
								toBeRemoved = true;
								break;
							}
						}
					}
				}
			}

/*			if (!toBeRemoved)
			{
				std::vector<ImplicitGeo::Triangle> simplexTriangles;
				for (int j = 0; j < 4; ++j)
				{
					simplexTriangles.push_back(ImplicitGeo::Triangle(simplexVertices[j], simplexVertices[(j + 1) % 4], simplexVertices[(j + 2) % 4]));
				}

				for (const auto& surfaceTriangle : surfaceTriangles)
				{
					if (LinearAlgebraUtils::isPointInsideSimplex(simplexVertices, surfaceTriangle.point1) ||
						LinearAlgebraUtils::isPointInsideSimplex(simplexVertices, surfaceTriangle.point2) ||
						LinearAlgebraUtils::isPointInsideSimplex(simplexVertices, surfaceTriangle.point3) ||
						surfaceTriangle.intersects(simplexTriangles[0]) || surfaceTriangle.intersects(simplexTriangles[1]) ||
						surfaceTriangle.intersects(simplexTriangles[2]) || surfaceTriangle.intersects(simplexTriangles[3]))
					{
						toBeRemoved = true;
						break;
					}
				}
			}*/

			if (toBeRemoved)
			{
				std::vector<TopologicalStruct*> simplexFacetsVec;
				std::unordered_set<TopologicalStruct*> simplexFacetsSet;
				currentSimplex->getAllChildren(simplexFacetsVec, simplexFacetsSet);

				for (const auto& facet : simplexFacetsVec)
				{
					auto halfFacet = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet);

					if (halfFacet->twin != nullptr)
					{
						if (contourSet.insert(halfFacet->twin->belongsTo).second)
						{
							contourQueue.push(halfFacet->twin->belongsTo);
						}
					}
				}

				manifold->erase3(currentSimplex);
			}
		}

		auto tetrahedraPoints = manifold->map3;

		glm::vec3 centroid = (vec3(-1, 0, 0) + vec3(1, 0, 0) + vec3(0, 1, 0) + vec3(0, 0, 1)) / 4.0f;
		vec4 v1(-1, 0, 0, 1);
		vec4 v2(1, 0, 0, 1);
		vec4 v3(0, 1, 0, 1);
		vec4 v4(0, 0, 1, 1);

//		auto triangles = DiscreteGeometryUtils::getTrianglesFromMesh(geometries["SURFACE"]);

		for (auto& simplex : tetrahedraPoints)
		{
			std::vector<TopologicalStruct*> verticesVec;
			std::unordered_set<TopologicalStruct*> verticesSet;
			simplex.second->getAllNthChildren(verticesVec, verticesSet, 0);
			std::vector<GLuint> volumeIndices;
			vec4 tetra;
			bool skip = false;
			for (int i = 0; i < 4; ++i)
			{
				tetra[i] = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(verticesVec[i])->halfSimplexData;
				volumeIndices.push_back(tetra[i]);
			}

			if (skip)
			{
				continue;
			}

			auto transform = LinearAlgebraUtils::getTransformFrom4Points(
				positions[volumeIndices[0]],
				positions[volumeIndices[1]],
				positions[volumeIndices[2]],
				positions[volumeIndices[3]]) *
				translate(mat4(1.0f), centroid) *
				scale(mat4(1.0f), vec3(1.0f)) *
				translate(mat4(1.0f), -centroid);

			tetraTransforms.push_back(transform);
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

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, meshObject, tetraTransforms.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;
	std::vector<glm::vec4> tetraColor;
	for (int i = 0; i < tetraTransforms.size(); ++i)
	{
		selectedC.push_back(1);
		tetraColor.push_back(vec4(0, 0, 1, 1));
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto volumeTransforms = new MatrixInstancedMeshObject<mat4, float>(selectable, tetraTransforms, "TETRAINSTANCES", 1);

	geometries["VOLUMES"] = new InstancedMeshObject<glm::vec4, float>(volumeTransforms, tetraColor, "TETRACOLORS", 1);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["VOLUMES"], "A2");
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