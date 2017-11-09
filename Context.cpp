#include "Context.h"
#include "WindowContext.h"
#include "ImplicitGeometry.h"
#include <omp.h>

AbstractContext* AbstractContext::activeContext = nullptr;

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

	makeQuad();
}

void SurfaceViewContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass(ShaderProgramPipeline::getPipeline("A"));
	gP->addRenderableObjects(geometries[0]);
	gP->setupCamera(cameras[0]);

	LightPass* lP = new LightPass(ShaderProgramPipeline::getPipeline("B"), true);
	lP->addRenderableObjects(geometries[1]);
	gP->addNeighbor(lP);

	passRootNode = gP;
}

PointSamplingContext::PointSamplingContext(DecoratedGraphicsObject* surface, SphericalCamera* cam)
{
	cameras.push_back(cam);
	geometries.push_back(surface);
	setupGeometries();
	setupPasses();
}

void PointSamplingContext::setupCameras(void)
{
}

void PointSamplingContext::setupGeometries(void)
{
	auto m = new Polyhedron(10, vec3(), vec3(1.0f));
	
	auto s = sampleSurface(10000);
	vector<mat4> transform;
	vec3 size(0.02f);

	for (int i = 0; i < s.size(); i++)
	{
		transform.push_back(translate(mat4(1.0f), s[i]) * scale(mat4(1.0f), size));
	} 

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, transform, "OFFSET");

	vector<GLuint> instanceID;

	for (int i = 1; i <= transform.size(); i++)
	{
		instanceID.push_back(i);
	}

	auto pickable = new InstancedMeshObject<GLuint, GLuint>(g, instanceID, "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	makeQuad();
}

void PointSamplingContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass(ShaderProgramPipeline::getPipeline("C"));
	gP->addRenderableObjects(geometries[1]);
	gP->setupCamera(cameras[0]);

	LightPass* lP = new LightPass(ShaderProgramPipeline::getPipeline("B"), true);
	lP->addRenderableObjects(geometries[2]);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

vector<vec3> PointSamplingContext::sampleSurface(int sampleSize)
{
	vector<vec3> samples;

	auto vertexBuffer = (MeshObject*)geometries[0]->signatureLookup("VERTEX");
	auto vertices = vertexBuffer->vertices;
	auto indices = vertexBuffer->indices;
	auto transformBuffer = (MatrixInstancedMeshObject<mat4, float>*)geometries[0]->signatureLookup("TRANSFORM");
	auto transforms = transformBuffer->extendedData;
	
	vector<Triangle> triangles;
	vec3 dir(1, 0, 0);
	vec3 min(INFINITY);
	vec3 max(-INFINITY);

	for (int i = 0; i < transforms.size(); i++)
	{
		mat4 t = transforms[i];
		vector<vec3> transformedPts;
		#pragma omp parallel for schedule(dynamic, 10)
		for (int j = 0; j < vertices.size(); j++)
		{
			transformedPts.push_back(vec3(t * vec4(vertices[j].position, 1)));

			for (int k = 0; k < 3; k++)
			{
				if (transformedPts[transformedPts.size() - 1][k] < min[k])
				{
					min[k] = transformedPts[transformedPts.size() - 1][k];
				}
				else if (transformedPts[transformedPts.size() - 1][k] > max[k])
				{
					max[k] = transformedPts[transformedPts.size() - 1][k];
				}
			}
		}
		
		#pragma omp parallel for schedule(dynamic, 10)
		for (int j = 0; j < indices.size(); j += 3)
		{
			triangles.push_back(Triangle(transformedPts[indices[j]], transformedPts[indices[j + 1]], transformedPts[indices[j + 2]]));
		}
	}

	vec3 boundSize = max - min;
	vec3 center = (max + min) / 2.0f;

	#pragma omp parallel for schedule(dynamic, 500)
	for (int i = 0; i < sampleSize;)
	{
		vec3 point(((float)rand()) / RAND_MAX - 0.5f, ((float)rand()) / RAND_MAX - 0.5f, ((float)rand()) / RAND_MAX - 0.5f);
		point *= boundSize;
		point += center;

		int sum = 0;
		for (int j = 0; j < triangles.size(); j++)
		{
			if (triangles[j].intersection(point, dir) > 0.0f)
			{
				sum++;
			}
		}

		if (sum % 2 == 1)
		{
			#pragma omp critical
			{
				samples.push_back(point);
				i++;
			}
		}
	}

	return samples;
}