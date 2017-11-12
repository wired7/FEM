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
	auto m = new ImportedMeshObject("models\\chinchilla.obj");

	vector<mat4> transform;
	/*	int number = 5;

	for (int k = 0; k <= number; k++)
	{
	for (int j = -number; j <= number; j++)
	{
	for (int i = -number; i <= number; i++)
	{
	vec3 pos = 15.0f * vec3(i, j, -2.0f * k);*/
	vec3 pos = vec3(-5, 1, 0);
	transform.push_back(scale(mat4(1.0f), vec3(0.4f)) * translate(mat4(1.0f), pos));
	/*				pos = vec3(5, 0, 0);
	transform.push_back(scale(mat4(1.0f), vec3(0.4f)) * translate(mat4(1.0f), pos));*/
	/*			}
	}
	}*/

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, transform, "TRANSFORM");

	vector<vec4> teamColor;
	//	int numColors = 20;
	//	for (int i = 0; i < numColors; i++)
	//		teamColor.push_back(vec4(((float)i) / numColors, 1, 1 - ((float)i) / numColors, 1));
	teamColor.push_back(vec4(0, 0, 1, 1));
	auto e = new InstancedMeshObject<vec4, float>(g, teamColor, "COLOR", transform.size() / teamColor.size());

	vector<GLuint> instanceID;

	for (int i = 1; i <= transform.size(); i++)
	{
		instanceID.push_back(i);
	}

	auto pickable = new InstancedMeshObject<GLuint, GLuint>(e, instanceID, "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	new HalfEdge::HalfSimplices(m->indices, 3);

	auto cylinder = new Cylinder(10);

	vector<mat4> transformC;
	mat4 s = scale(mat4(1.0f), vec3(1, 0.005f, 0.005f));

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < m->indices.size(); i++)
		{
			vec3 point[2];

			point[0] = vec3(transform[j] * vec4(m->vertices[m->indices[i]].position, 1));
			point[1] = vec3(transform[j] * vec4(m->vertices[m->indices[3 * (i / 3) + ((i + 1) % 3)]].position, 1));
			vec3 z = -normalize(cross(normalize(point[1] - point[0]), vec3(1, 0, 0)));
			float angle = acos(dot(normalize(point[1] - point[0]), vec3(1, 0, 0)));
			transformC.push_back(translate(mat4(1.0f), point[0]) * rotate(mat4(1.0f), angle, z) * scale(mat4(1.0f), vec3(length(point[1] - point[0]), 1, 1)) * s);
		}
	}

	g = new MatrixInstancedMeshObject<mat4, float>(cylinder, transformC, "TRANSFORM");

	vector<vec4> teamColorC;
	teamColorC.push_back(vec4(1, 1, 1, 1));
	e = new InstancedMeshObject<vec4, float>(g, teamColorC, "COLOR", transformC.size() / teamColorC.size());

	vector<GLuint> instanceIDC;

	for (int i = 1; i <= transformC.size(); i++)
	{
		instanceIDC.push_back(i);
	}

	pickable = new InstancedMeshObject<GLuint, GLuint>(e, instanceIDC, "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries.push_back(selectable);

	makeQuad();
}

void SurfaceViewContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->addRenderableObjects(geometries[1], 0);
	gP->setupCamera(cameras[0]);

	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[2], 0);
	gP->addNeighbor(lP);

	passRootNode = gP;
}