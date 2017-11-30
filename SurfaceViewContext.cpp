#include "SurfaceViewContext.h"
#include "HalfSimplices.h"

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

//	auto m = new ImportedMeshObject("models\\chinchilla.obj");
	auto m = new Cylinder(10);//Polyhedron(10, vec3(), vec3(1.0f));

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
	
	HalfEdge::HalfSimplices * hSimp = new HalfEdge::HalfSimplices(m->indices, 3);

	auto cylinder = new Arrow();

	vector<mat4> transformC;
	vector<vec3> centroids;
	vector<GLbyte> warningC;

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < hSimp->halfEdges.size(); i++)
		{
			warningC.push_back(0);

			vec3 centroid(0.0f);
			int count = 0;
			HalfEdge::HalfEdge* halfEdge = hSimp->halfEdges[i];
			HalfEdge::HalfEdge* newEdge = halfEdge;
			while (true) {
				centroid += m->vertices[newEdge->vertex->externalIndex].position;
				count++;
				newEdge = newEdge->next;
				if (newEdge == halfEdge)
				{
					break;
				}
			}

			centroid /= count;

			vec3 point[2];
			point[0] = vec3(transform[j] * vec4(m->vertices[hSimp->halfEdges[i]->start].position, 1));
			point[1] = vec3(transform[j] * vec4(m->vertices[hSimp->halfEdges[i]->end].position, 1));
			float edgeLength = length(point[1] - point[0]);
			vec3 z = -normalize(cross(normalize(point[1] - point[0]), vec3(1, 0, 0)));
			float angle = acos(dot(normalize(point[1] - point[0]), vec3(1, 0, 0)));
			transformC.push_back(
								scale(mat4(1.0f), vec3(1.0f / 0.65f)) * translate(mat4(1.0f), centroid) * 
								scale(mat4(1.0f), vec3(0.5f)) * 
								translate(mat4(1.0f), point[0] - centroid) * 
								rotate(mat4(1.0f), angle, z) * 
								scale(mat4(1.0f), vec3(edgeLength, edgeLength * 0.01f, edgeLength * 0.01f)) * 
								translate(mat4(1.0f), vec3(0.5f, 0, 0)) *
								scale(mat4(1.0f), vec3(0.9f, 1, 1)) *
								translate(mat4(1.0f), vec3(-0.5f, 0, 0)));
		}

		for (int i = 0; i < hSimp->holes.size(); i++)
		{
			
			vec3 centroid(0.0f);
			int count = 0;
			HalfEdge::HalfEdge* halfEdge = hSimp->holes[i]->halfEdge;
			HalfEdge::HalfEdge* newEdge = halfEdge;
			while (true) {
				centroid += m->vertices[newEdge->vertex->externalIndex].position;
				count++;
				newEdge = newEdge->next;
				if (newEdge == halfEdge)
				{
					break;
				}
			}

			centroid /= count;

			newEdge = halfEdge;
			while (true) {
				warningC.push_back(1);
				vec3 point[2];
				point[0] = vec3(transform[j] * vec4(m->vertices[newEdge->start].position, 1));
				point[1] = vec3(transform[j] * vec4(m->vertices[newEdge->end].position, 1));
				float edgeLength = length(point[1] - point[0]);
				vec3 z = -normalize(cross(normalize(point[1] - point[0]), vec3(1, 0, 0)));
				float angle = acos(dot(normalize(point[1] - point[0]), vec3(1, 0, 0)));
				transformC.push_back(
					scale(mat4(1.0f), vec3(1.0f / 0.65f)) * translate(mat4(1.0f), centroid) *
					scale(mat4(1.0f), vec3(0.5f)) *
					translate(mat4(1.0f), point[0] - centroid) *
					rotate(mat4(1.0f), angle, z) *
					scale(mat4(1.0f), vec3(edgeLength, edgeLength * 0.01f, edgeLength * 0.01f)) *
					translate(mat4(1.0f), vec3(0.5f, 0, 0)) *
					scale(mat4(1.0f), vec3(0.9f, 1, 1)) *
					translate(mat4(1.0f), vec3(-0.5f, 0, 0)));

				newEdge = newEdge->next;
				if (newEdge == halfEdge)
				{
					break;
				}
			}
		}
	}

	

	g = new MatrixInstancedMeshObject<mat4, float>(cylinder, transformC, "TRANSFORM");

	pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, transformC.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto highlightable = new InstancedMeshObject<GLbyte, GLbyte>(selectable, warningC, "WARNING", 1);

	geometries.push_back(highlightable);

	makeQuad();
}

void SurfaceViewContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A"), ShaderProgramPipeline::getPipeline("EdgeA") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->addRenderableObjects(geometries[1], 1);
	gP->setupCamera(cameras[0]);

	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[2], 0);
	gP->addNeighbor(lP);

	passRootNode = gP;
}