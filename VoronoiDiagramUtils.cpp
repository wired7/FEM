#include "VoronoiDiagramUtils.h"
#include <map>

Sphere VoronoiDiagramUtils::getCircumcircle(vec3 points[3])
{
	// center is the point equidistant from 3 non-colinear vertices
	vec3 edgeDir[2];

	for (int i = 0; i < 2; i++)
	{
		edgeDir[i] = points[i + 1] - points[i];
	}

	vec3 normal = normalize(cross(normalize(edgeDir[0]), normalize(edgeDir[1])));

	vec3 perpDir[2];
	vec3 midPoint[2];

	for (int i = 0; i < 2; i++)
	{
		perpDir[i] = normalize(cross(normal, normalize(edgeDir[i])));
		midPoint[i] = edgeDir[i] / 2.0f + points[i];
	}

	float radius;
	for (int i = 0; i < 3; i++)
	{
		float diff = perpDir[1][i] - perpDir[0][i];
		if (diff != 0.0f)
		{
			radius = (midPoint[1][i] - midPoint[0][i]) / diff;
			break;
		}
	}

	return Sphere(midPoint[0] + radius * perpDir[0], length(midPoint[0] + radius * perpDir[0] - points[0]));
}

Sphere VoronoiDiagramUtils::getCircumsphere(vec3 points[4])
{
	// center is the point equidistant from 4 non-coplanar vertices

	Sphere circumCircles[2] = { getCircumcircle(points), getCircumcircle(&points[1]) };

	vec3 edgeDirs[3];

	for (int i = 0; i < 3; i++)
	{
		edgeDirs[i] = points[i + 1] - points[i];
	}

	vec3 normals[2];

	for (int i = 0; i < 2; i++)
	{
		normals[i] = normalize(cross(normalize(edgeDirs[i]), normalize(edgeDirs[i + 1])));
	}

	float radius;
	for (int i = 0; i < 3; i++)
	{
		float diff = normals[1][i] - normals[0][i];
		if (diff != 0.0f)
		{
			radius = (circumCircles[1].center[i] - circumCircles[0].center[i]) / diff;
			break;
		}
	}

	return Sphere(circumCircles[0].center + normals[0] * radius, length(circumCircles[0].center + normals[0] * radius - points[0]));
}

Geometry::Vertex* VoronoiDiagramUtils::getVoronoiPointFromTetrahedron(Geometry::Mesh* mesh, const vector<vec3>& inputPositions, vector<vec3>& outputPositions)
{
	auto vertices = HalfEdgeUtils::getVolumeVertices(mesh, inputPositions);

	if (vertices.size() != 4)
	{
		cout << "INVALID NUMBER OF VERTICES IN VOLUME" << endl;
		system("PAUSE");

		return nullptr;
	}

	Geometry::Vertex* outputVertex = new Geometry::Vertex(outputPositions.size());

	vec3 outputPos = getCircumsphere(&(vertices[0])).center;
	outputPositions.push_back(outputPos);
	
	return outputVertex;
}

pair<Geometry::Vertex*, Geometry::Vertex*> VoronoiDiagramUtils::getVoronoiEdgeFromTetrahedraPair(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2, const map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices)
{
	auto point1 = voronoiVertices.find(mesh1)->second;
	auto point2 = voronoiVertices.find(mesh2)->second;

	return pair<Geometry::Vertex*, Geometry::Vertex*>(point1, point2);
}

Geometry::Facet* VoronoiDiagramUtils::getVoronoiFacetFromEdge(pair<Geometry::Vertex*, Geometry::Vertex*> edgeVertices, const map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices)
{
	auto meshes = HalfEdgeUtils::getEdgeMeshes(edgeVertices);
	vector<pair<Geometry::Vertex*, Geometry::Vertex*>> edges;

	for (int i = 0; i < meshes.size() - 1; i++)
	{
		for (int j = 0; j < meshes[i]->facets.size(); j++)
		{
			for (int k = i + 1; k < meshes.size(); k++)
			{
				for (int l = 0; l < meshes[k]->facets.size(); l++)
				{
					if (meshes[i]->facets[j]->twin == meshes[k]->facets[l])
					{
						edges.push_back(getVoronoiEdgeFromTetrahedraPair(meshes[i], meshes[k], voronoiVertices));
					}
				}
			}
		}
	}
	return nullptr;
}

Geometry::VolumetricMesh* VoronoiDiagramUtils::getVoronoiDiagram(Geometry::VolumetricMesh* volumetricMesh, const vector<vec3>& positions)
{
	map<Geometry::Mesh*, Geometry::Vertex*> voronoiVertices;
	vector<vec3> outputPositions;

	for (int i = 0; i < volumetricMesh->meshes.size(); i++)
	{
		voronoiVertices[volumetricMesh->meshes[i]] = getVoronoiPointFromTetrahedron(volumetricMesh->meshes[i], positions, outputPositions);
	}
	return nullptr;
	
}