#include "HalfSimplexRenderingUtils.h"
#include <unordered_map>
#include "LinearAlgebraUtils.h"

namespace std {
	template <> struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& vector) const
		{
			return hash<float>()(vector.x * vector.y * vector.z + vector.y * vector.z + vector.z);
		}
	};
};

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableVolumesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							  const vector<vec3>& positions,
																							  const vector<mat4>& transforms,
																							  ReferenceManager* refMan)
{
	vector<GLuint> indices;
	vector<Graphics::Vertex> vertices;

	std::unordered_map<glm::vec3, Graphics::Vertex> vertexMap;
	std::vector<glm::vec3> indexIter;

	std::vector<Geometry::HalfSimplex<2, GLuint>*> facets = manifold->getHalfSimplices2();

	for (const auto& transform : transforms)
	{
		for (const auto& facet : facets)
		{
			std::unordered_set<Geometry::TopologicalStruct*> facetVertices;
			facet->getAllNthChildren(facetVertices, 0);

			std::vector<glm::vec3> facetVertexPositions;
			for (const auto& facetVertex : facetVertices)
			{
				auto fV = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetVertex);
				vec3 pos = vec3(transform * vec4(positions[fV->halfSimplexData], 1));
				auto neighbours = fV->getNeighbours();

				std::vector<glm::vec3> adjacencyLines;
				for (const auto& neighbour : neighbours)
				{
					adjacencyLines.push_back(pos - vec3(transform * vec4(positions[neighbour.second->halfSimplexData], 1)));
				}

				glm::vec3 normal;
				for (int i = 0; i < adjacencyLines.size(); ++i)
				{
					normal += cross(adjacencyLines[i], adjacencyLines[i % adjacencyLines.size()]);
				}
				
				vertexMap[pos] = Graphics::Vertex(pos, normalize(normal));
				facetVertexPositions.push_back(pos);
			}
			
			// TODO: triangulate facetVertices and iterate through all the new facets inside this context
			for(const auto& fVPos : facetVertexPositions)
			{
				indexIter.push_back(fVPos);
			}
		}

	}

	std::unordered_map<glm::vec3, int> vertexIndexMap;
	for (const auto& vertex : vertexMap)
	{
		vertices.push_back(vertex.second);
		vertexIndexMap[vertex.second.position] = vertices.size() - 1;
	}

	for (const auto& index : indexIter)
	{
		indices.push_back(vertexIndexMap[index]);
	}

	auto meshObject = new Graphics::MeshObject(vertices, indices);

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, meshObject, indices.size() / 3, "INSTANCEID", 3);

	vector<GLbyte> selectedC;

	for (int i = 0; i < indices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::ExtendedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION");

	vector<mat4> parentTransforms;

	parentTransforms.push_back(scale(mat4(1.0f), vec3(1.001f)));

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM");

	return g;
}

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableFacetsFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							  const vector<vec3>& positions,
																							  const vector<mat4>& transforms,
																							  ReferenceManager* refMan)
{
	vector<vec3> centroids;
	vector<glm::mat4> triangleTransforms;

	std::vector<Geometry::HalfSimplex<2, GLuint>*> facets = manifold->getHalfSimplices2();
	std::vector<Geometry::HalfSimplex<0, GLuint>*> verts = manifold->getHalfSimplices0();

	vec3 volumeCentroid;

	for (const auto& vertex : verts)
	{
		volumeCentroid += positions[vertex->halfSimplexData];
	}

	volumeCentroid /= (float)verts.size();

	for (const auto& transform : transforms)
	{
		mat4 volumeTransform = transform * translate(mat4(1.0f), volumeCentroid) * scale(mat4(1.0f), vec3(1.01f)) *
			translate(mat4(1.0f), -volumeCentroid);

		for (const auto& facet : facets)
		{
			glm::vec3 facetCentroid;
			std::vector<glm::vec3> vertices;
			Geometry::HalfSimplex<1, GLuint>* currentEdge = facet->pointsTo;
			Geometry::HalfSimplex<1, GLuint>* firstEdge = currentEdge;
			do
			{
				vec3 pos = vec3(transform * vec4(positions[currentEdge->pointsTo->halfSimplexData], 1));

				vertices.push_back(pos);
				facetCentroid += pos;
				currentEdge = currentEdge->next;
			} while (currentEdge != firstEdge);

			// TODO: Tesselate if more than 3 points in facet

			triangleTransforms.push_back(translate(mat4(1.0f), facetCentroid) *
										 scale(mat4(1.0f), vec3(0.95f)) *										
										 translate(mat4(1.0f), -facetCentroid) * 
										 LinearAlgebraUtils::getTransformFrom3Points(vertices[0], vertices[1], vertices[2]));
		}
	}

	auto meshObject = new Triangle();

	auto instancedTriangles = new MatrixInstancedMeshObject<mat4, float>(meshObject, triangleTransforms, "TRIANGLEINSTANCES", 1);

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, instancedTriangles, triangleTransforms.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < triangleTransforms.size(); ++i)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	vector<mat4> parentTransforms;

	parentTransforms.push_back(scale(mat4(1.0f), vec3(0.91f)));

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(selectable, parentTransforms, "TRANSFORM", triangleTransforms.size());

	return g;
}

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableEdgesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							 const vector<vec3>& positions,
																							 const vector<mat4>& transforms,
																							 ReferenceManager* refMan)
{
	vector<mat4> transformC;
	vector<vec3> centroids;
	vector<GLbyte> warningC;

	std::vector<Geometry::HalfSimplex<2, GLuint>*> facets = manifold->getHalfSimplices2();
	std::vector<Geometry::HalfSimplex<0, GLuint>*> verts = manifold->getHalfSimplices0();

	vec3 volumeCentroid;

	for (const auto& vertex : verts)
	{
		volumeCentroid += positions[vertex->halfSimplexData];
	}

	volumeCentroid /= (float)verts.size();

	for (const auto& transform : transforms)
	{
/*		mat4 volumeTransform = transform * translate(mat4(1.0f), volumeCentroid) * scale(mat4(1.0f), vec3(1.01f)) *
			translate(mat4(1.0f), -volumeCentroid);*/

		for (const auto& facet : facets)
		{
			std::unordered_set<Geometry::TopologicalStruct*> facetVertices;
			facet->getAllNthChildren(facetVertices, 0);

			glm::vec3 facetCentroid;
			for (const auto& facetVertex : facetVertices)
			{
				auto fV = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetVertex);
				facetCentroid += vec3(transform * vec4(positions[fV->halfSimplexData], 1));
			}

			facetCentroid /= facetVertices.size();

			std::unordered_set<Geometry::TopologicalStruct*> facetEdges;
			facet->getAllNthChildren(facetEdges, 1);
			for (const auto& edge : facetEdges)
			{
				warningC.push_back(0);
				transformC.push_back(getHalfEdgeTransform(reinterpret_cast<Geometry::HalfSimplex<1, GLuint>*>(edge),
														  positions, transform, facetCentroid));
			}
		}
	}

	auto arrow = new Arrow();

	auto g = new Graphics::MatrixInstancedMeshObject<mat4, float>(arrow, transformC, "EDGETRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, transformC.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto highlightable = new InstancedMeshObject<GLbyte, GLbyte>(selectable, warningC, "WARNING", 1);

	return highlightable;
}

mat4 HalfSimplexRenderingUtils::getHalfEdgeTransform(Geometry::HalfSimplex<1, GLuint>* halfEdge,
													 const vector<vec3>& positions,
													 const mat4& parentTransform,
													 const vec3 centroid)
{
	vec3 point[2];
	point[0] = vec3(parentTransform * vec4(positions[halfEdge->pointsTo->halfSimplexData], 1));
	point[1] = vec3(parentTransform * vec4(positions[halfEdge->previous->pointsTo->halfSimplexData], 1));
	float edgeLength = length(point[1] - point[0]);
	auto lineTransform = LinearAlgebraUtils::getTransformFrom2Points(point[0], point[1]);

	mat4 aroundCentroid =
		translate(mat4(1.0f), -centroid) *
		lineTransform *
		scale(mat4(1.0f), vec3(1, 0.002f, 0.002f)) *
		translate(mat4(1.0f), vec3(0.5f, 0, 0)) *
		scale(mat4(1.0f), vec3(0.9f)) *
		translate(mat4(1.0f), vec3(-0.5f, 0, 0));

	mat4 transform =
		scale(mat4(1.0f), vec3(1.001f)) *
		translate(mat4(1.0f), centroid) *
		scale(mat4(1.0f), vec3(0.7f)) *
		aroundCentroid;

	return transform;
}