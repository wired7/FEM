#include "Context.h"
#include "WindowContext.h"
#include "ImplicitGeometry.h"
#include "HalfSimplices.h"
#include <thread>
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

PointSamplingContext::PointSamplingContext(DecoratedGraphicsObject* surface, SphericalCamera* cam)
{
	cameras.push_back(cam);
	geometries.push_back(surface);
	setupPasses();

	thread t([=] {
		points = sampleSurface(10000, surface);
		pointsReady = true;
	});
	t.detach();
}

void PointSamplingContext::setupGeometries(void)
{
	((GeometryPass*)passRootNode)->clearRenderableObjects(1);

	auto m = new Polyhedron(10, vec3(), vec3(1.0f));
	
	vector<mat4> transform;
	vec3 size(0.02f);

	for (int i = 0; i < points.size(); i++)
	{
		transform.push_back(translate(mat4(1.0f), points[i]) * scale(mat4(1.0f), size));
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

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, 1);
	dirty = true;
}

void PointSamplingContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("A"), ShaderProgramPipeline::getPipeline("C") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void PointSamplingContext::update(void)
{
	GraphicsSceneContext<PointSamplingController, SphericalCamera, PointSamplingContext>::update();

	if (pointsReady)
	{
		setupGeometries();
		pointsReady = false;
	}
}

vector<vec3> PointSamplingContext::sampleSurface(int sampleSize, DecoratedGraphicsObject* object)
{
	vector<vec3> samples;

	auto vertexBuffer = (MeshObject*)object->signatureLookup("VERTEX");
	auto vertices = vertexBuffer->vertices;
	auto indices = vertexBuffer->indices;
	auto transformBuffer = (MatrixInstancedMeshObject<mat4, float>*)object->signatureLookup("TRANSFORM");
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

TetrahedralizationContext::TetrahedralizationContext(DecoratedGraphicsObject* surface, vector<vec3> points, SphericalCamera* cam)
{
	cameras.push_back(cam);
	geometries.push_back(surface);
	setupPasses();

	thread t([=] {
		// compute tetrahedralization here
		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{
	((GeometryPass*)passRootNode)->clearRenderableObjects(0);

	auto m = new Tetrahedron();

	auto g = new MatrixInstancedMeshObject<mat4, float>(m, tetrahedra, "OFFSET");

	vector<GLuint> instanceID;

	for (int i = 1; i <= tetrahedra.size(); i++)
	{
		instanceID.push_back(i);
	}

	auto pickable = new InstancedMeshObject<GLuint, GLuint>(g, instanceID, "INSTANCEID", 1);

	vector<GLbyte> selected;

	for (int i = 0; i < tetrahedra.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries.push_back(selectable);

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, 0);
}

void TetrahedralizationContext::setupPasses(void)
{
	// TODO: might want to manage passes as well
	GeometryPass* gP = new GeometryPass({ ShaderProgramPipeline::getPipeline("C") });
	gP->addRenderableObjects(geometries[0], 0);
	gP->setupCamera(cameras[0]);

	makeQuad();
	LightPass* lP = new LightPass({ ShaderProgramPipeline::getPipeline("B") }, true);
	lP->addRenderableObjects(geometries[1], 0);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

void TetrahedralizationContext::update(void)
{
	GraphicsSceneContext<TetrahedralizationController, SphericalCamera, TetrahedralizationContext>::update();

	if (tetrahedralizationReady)
	{
		setupGeometries();
		tetrahedralizationReady = false;
	}
}