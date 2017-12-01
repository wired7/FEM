#include "SurfaceViewContext.h"
#include "HalfEdgeUtils.h"

using namespace Graphics;
SurfaceViewContext::SurfaceViewContext() : GraphicsSceneContext()
{
	setupCameras();
	setupGeometries();
	setupPasses();
}

void SurfaceViewContext::setupCameras(void)
{
	int width, height;
	glfwGetWindowSize(WindowContext::window, &width, &height);

	cameras.push_back(new SphericalCamera(WindowContext::window, vec2(0, 0), vec2(1, 1), vec3(0, 0, 5), vec3(0, 0, 0),
		vec3(0, 1, 0), perspective(45.0f, (float)width / height, 0.1f, 1000.0f)));
}

void SurfaceViewContext::setupGeometries(void)
{
	refMan = new ReferenceManager();
	auto m = new ImportedMeshObject("models\\filledChinchilla.obj");
//	auto m = new Cylinder(10);//Polyhedron(10, vec3(), vec3(1.0f));

	vector<mat4> transform;

	vec3 pos = vec3(0, 0, 0);
	transform.push_back(scale(mat4(1.0f), vec3(3.0f)) * translate(mat4(1.0f), pos));

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, transform, "TRANSFORM");

	vector<vec4> teamColor;
	teamColor.push_back(vec4(0, 0, 1, 1));

	auto e = new InstancedMeshObject<vec4, float>(g, teamColor, "COLOR", transform.size() / teamColor.size());
	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, e, transform.size(), "INSTANCEID", 1);

	vector<GLbyte> selected;
	// Maybe have an instanced mesh object constructor with a size and an initializer if all values will be the same
	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(0);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Geometry::Mesh * hSimp = new Geometry::Mesh(m->indices, 3);

	setupRenderableHalfEdges(hSimp, selectable);
	setupRenderableVertices(selectable);
	setupRenderableFacets(hSimp, selectable);

	makeQuad();
}

void SurfaceViewContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A"), ShaderProgramPipeline::getPipeline("EdgeA"), ShaderProgramPipeline::getPipeline("C"), ShaderProgramPipeline::getPipeline("D")});
	gP->addRenderableObjects(geometries[0], 0);
	gP->addRenderableObjects(geometries[1], 1);
	gP->addRenderableObjects(geometries[2], 2);
	gP->addRenderableObjects(geometries[3], 3);
	gP->setupCamera(cameras[0]);

	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[4], 0);
	gP->addNeighbor(lP);

	passRootNode = gP;
}

void SurfaceViewContext::setupRenderableHalfEdges(Geometry::Mesh* hSimp, DecoratedGraphicsObject* o)
{
	auto cylinder = new Arrow();

	auto transform = ((InstancedMeshObject<mat4, float>*)o->signatureLookup("TRANSFORM"))->extendedData;
	auto m = (MeshObject*)o->signatureLookup("VERTEX");

	vector<mat4> transformC;
	vector<vec3> centroids;
	vector<GLbyte> warningC;

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < hSimp->facets.size(); i++)
		{
			vec3 centroid = HalfEdgeUtils::getFacetCentroid(hSimp->facets[i], m, transform[j]);
			auto edges = HalfEdgeUtils::getFacetHalfEdges(hSimp->facets[i]);

			for (int k = 0; k < edges.size(); k++)
			{
				warningC.push_back(0);
				transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], m, transform[j], centroid));
			}
		}

		for (int i = 0; i < hSimp->holes.size(); i++)
		{
			vec3 centroid = HalfEdgeUtils::getFacetCentroid(hSimp->holes[i], m, transform[j]);

			auto edges = HalfEdgeUtils::getFacetHalfEdges(hSimp->holes[i]);

			for (int k = 0; k < edges.size(); k++)
			{
				warningC.push_back(1);
				transformC.push_back(HalfEdgeUtils::getHalfEdgeTransform(edges[k], m, transform[j], centroid));
			}
		}
	}

	auto g = new MatrixInstancedMeshObject<mat4, float>(cylinder, transformC, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, transformC.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto highlightable = new InstancedMeshObject<GLbyte, GLbyte>(selectable, warningC, "WARNING", 1);

	geometries.push_back(highlightable);
}

void SurfaceViewContext::setupRenderableVertices(DecoratedGraphicsObject* o)
{
	auto point = new Polyhedron(4, vec3(), vec3(1.0f));

	auto transform = ((InstancedMeshObject<mat4, float>*)o->signatureLookup("TRANSFORM"))->extendedData;
	auto vertices = ((MeshObject*)o->signatureLookup("VERTEX"))->vertices;
	vector<mat4> positions;

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			positions.push_back(transform[j] * translate(mat4(1.0f), vertices[i].position) * scale(mat4(1.0f), vec3(0.02f)));
		}
	}

	auto g = new MatrixInstancedMeshObject<mat4, float>(point, positions, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, vertices.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < vertices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries.push_back(selectable);
}

void SurfaceViewContext::setupRenderableFacets(Geometry::Mesh* hSimp, Graphics::DecoratedGraphicsObject* o)
{
	vector<GLuint> indices;
	vector<Vertex> vertices;
	auto transform = ((InstancedMeshObject<mat4, float>*)o->signatureLookup("TRANSFORM"))->extendedData;
	auto mesh = ((MeshObject*)o->signatureLookup("VERTEX"));

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < hSimp->facets.size(); i++)
		{
			auto facetVertices = HalfEdgeUtils::getFacetVertices(hSimp->facets[i]);
			vec3 centroid = HalfEdgeUtils::getFacetCentroid(hSimp->facets[i], mesh, transform[j]);
			vec3 normal;

			for (int k = 0; k < facetVertices.size(); k++)
			{
				normal += vec3(inverse(transpose(transform[j])) * vec4(mesh->vertices[facetVertices[k]->externalIndex].normal, 0));
			}
			
			normal = normalize(normal);
			vertices.push_back(Vertex(centroid, normal));
			int centroidIndex = vertices.size() - 1;

			for (int k = 0; k <= facetVertices.size(); k++)
			{
				vec3 pos = vec3(transform[j] * vec4(mesh->vertices[facetVertices[k % facetVertices.size()]->externalIndex].position, 1));

				pos = centroid + 0.7f * (pos - centroid);

				vertices.push_back(Vertex(pos, normal));

				if (k > 0)
				{
					indices.push_back(centroidIndex);
					indices.push_back(centroidIndex + k - 1);
					indices.push_back(centroidIndex + k);
				}
			}

			indices.push_back(centroidIndex);
			indices.push_back(centroidIndex + facetVertices.size());
			indices.push_back(centroidIndex + 1);
		}
	}

	auto meshObject = new MeshObject(vertices, indices);

	vector<mat4> positions;

	positions.push_back(scale(mat4(1.0f), vec3(1.001f)));

	auto g = new MatrixInstancedMeshObject<mat4, float>(meshObject, positions, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, vertices.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < vertices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries.push_back(selectable);
}