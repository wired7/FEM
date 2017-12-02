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
//	auto m = new Polyhedron(10, vec3(), vec3(1.0f));

	vector<mat4> transform;

	vec3 pos = vec3(0, 0, 0);
	transform.push_back(scale(mat4(1.0f), vec3(1.0f)) * translate(mat4(1.0f), pos));

	Geometry::Mesh * hSimp = new Geometry::Mesh(m->indices, 3);
	Geometry::VolumetricMesh* vMesh = new Geometry::VolumetricMesh();
	vMesh->meshes.push_back(hSimp);

	vector<vec3> positions;
	for (int i = 0; i < m->vertices.size(); i++)
	{
		positions.push_back(m->vertices[i].position);
	}

	auto outputGeometry = HalfEdgeUtils::getRenderableVolumesFromMesh(vMesh, positions, refMan, transform);

	auto instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)outputGeometry->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	int currentIndex = objectIDs[0];
	int currentManagedIndex = refMan->assignNewGUID();
	for (int i = 0; i < objectIDs.size(); i++)
	{
		if (objectIDs[i] != currentIndex)
		{
			currentIndex = objectIDs[i];
			currentManagedIndex = refMan->assignNewGUID();
		}

		objectIDs[i] = currentManagedIndex;
	}

	instanceIDs->extendedData = objectIDs;
	instanceIDs->updateBuffers();

	geometries.push_back(outputGeometry);

	setupRenderableHalfEdges(vMesh, positions);
	setupRenderableVertices(positions);
	setupRenderableFacets(vMesh, positions);

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

void SurfaceViewContext::setupRenderableHalfEdges(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions)
{
	auto outputGeometry = HalfEdgeUtils::getRenderableEdgesFromMesh(hSimp, positions, refMan);

	geometries.push_back(outputGeometry);
}

void SurfaceViewContext::setupRenderableVertices(const vector<vec3>& positions)
{
	vector<mat4> pTransforms;

	for (int i = 0; i < positions.size(); i++)
	{
		pTransforms.push_back(translate(mat4(1.0f), positions[i]) * scale(mat4(1.0f), vec3(0.02f)));
	}

	auto point = new Polyhedron(4, vec3(), vec3(1.0f));

	auto g = new MatrixInstancedMeshObject<mat4, float>(point, pTransforms, "TRANSFORM");

	auto pickable = new ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, pTransforms.size(), "INSTANCEID", 1);

	vector<GLbyte> selectedC;

	for (int i = 0; i < pTransforms.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries.push_back(selectable);
}

void SurfaceViewContext::setupRenderableFacets(Geometry::VolumetricMesh* hSimp, const vector<vec3>& positions)
{
	auto outputGeometry = HalfEdgeUtils::getRenderableFacetsFromMesh(hSimp, positions);

	auto instanceIDs = ((ExtendedMeshObject<GLuint, GLuint>*)outputGeometry->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	int currentIndex = objectIDs[0];
	int currentManagedIndex = refMan->assignNewGUID();
	for (int i = 0; i < objectIDs.size(); i++)
	{
		if (objectIDs[i] != currentIndex)
		{
			currentIndex = objectIDs[i];
			currentManagedIndex = refMan->assignNewGUID();
		}

		objectIDs[i] = currentManagedIndex;
	}

	instanceIDs->extendedData = objectIDs;
	instanceIDs->updateBuffers();

	geometries.push_back(outputGeometry);
}