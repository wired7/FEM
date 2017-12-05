#include "VoronoiDiagramUtils.h"
#include <map>

Sphere VoronoiDiagramUtils::getCircumcircle(vec3 points[3])
{
	// center is the point equidistant from 3 non-colinear vertices
	vec3 edgeDir[3];

	for (int i = 0; i <= 3; i++)
	{
		edgeDir[i] = points[(i + 1) % 3] - points[i];
	}

	vec3 normal = normalize(cross(normalize(edgeDir[0]), normalize(edgeDir[1])));

	if (length(normal) == 0.0f)
	{
		normal = normalize(cross(normalize(edgeDir[1]), normalize(edgeDir[2])));

		if (length(normal) == 0.0f)
		{
			normal = normalize(cross(normalize(edgeDir[0]), normalize(edgeDir[2])));
		}
	}

	vec3 perpDir[2];
	vec3 midPoint[2];

	for (int i = 0; i < 2; i++)
	{
		perpDir[i] = normalize(cross(normal, normalize(edgeDir[i])));
		midPoint[i] = edgeDir[i] / 2.0f + points[i];
	}

	float radius = 0;
	float diff = 0;

	for (int i = 0; i < 3; i++)
	{
		diff += pow(perpDir[1][i] - perpDir[0][i], 2.0f);
	}
	diff = sqrt(diff);
	for (int i = 0; i < 3; i++)
	{
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
	mat4 a(vec4(points[0], 1), vec4(points[1], 1), vec4(points[2], 1), vec4(points[3], 1));
	mat4 x[3];
	mat4 c;
	vec3 detx;
	double lengths[4];

	for (int j = 0; j < 4; j++)
	{
		lengths[j] = pow(length(points[j]), 2.0f);
	}

	int indices[3][2] = { {1, 2}, {0, 2}, {0, 1} };
	for (int i = 0; i < 3; i++)
	{		
		for (int j = 0; j < 4; j++)
		{
			x[i][j] = vec4(lengths[j], points[j][indices[i][0]], points[j][indices[i][1]], 1);
			c[j] = vec4(lengths[j], points[j][0], points[j][1], points[j][2]);
		}

		detx[i] = determinant(x[i]);
//		cout << detx[i] << endl;
	}

	detx[1] *= -1.0f;
	double detA = determinant(a);
	double detC = determinant(c);

	vec3 center = detx / (float)(2.0 * detA);
	double radius = sqrt(length(detx) - 4.0f * detA * detC) / (2.0f * abs(detA));
//	cout << detA << endl;
//	system("PAUSE");
	return Sphere(center, radius);
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