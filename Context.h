#pragma once
#include <glew.h>
#include "Camera.h"
#include "Controller.h"
#include "GeometricalMeshObjects.h"
#include "FrameBuffer.h"
#include "Pass.h"

using namespace std;

class AbstractContext
{
public:
	static AbstractContext* activeContext;
	virtual void update() = 0;
};

template<class ControllerType, class CameraType, class ContextType> class Context : public AbstractContext
{
public:
	ControllerType* controller;
	vector<CameraType*> cameras;
	AbstractContext* nextContext;
	AbstractContext* prevContext;
	Context();
	~Context();
	virtual void setAsActiveContext(void);
};

#pragma region ContextTemplate
template<class ControllerType, class CameraType, class ContextType> Context<ControllerType, CameraType, ContextType>::Context()
{

}

template<class ControllerType, class CameraType, class ContextType> Context<ControllerType, CameraType, ContextType>::~Context()
{
}

template<class ControllerType, class CameraType, class ContextType> void Context<ControllerType, CameraType, ContextType>::setAsActiveContext(void)
{
	activeContext = this;

	controller = ControllerType::getController();
	controller->setContext((ContextType*)this);
	controller->setController();
}
#pragma endregion

class Pass;

template<class ControllerType, class CameraType, class ContextType> class GraphicsSceneContext : public Context<ControllerType, CameraType, ContextType>
{
protected:
	virtual void setupCameras(void) = 0;
	virtual void setupGeometries(void) = 0;
	virtual void setupPasses(const std::vector<std::string>& gProgramSignatures,
							 const std::vector<std::string>& lProgramSignatures);
	virtual void makeQuad(void);
public:
	bool dirty = true;
	Pass* passRootNode;
	map<string, Graphics::DecoratedGraphicsObject*> geometries;
	GraphicsSceneContext() {};
	~GraphicsSceneContext() {};
	virtual void update(void);
};

#pragma region GraphicsSceneContextTemplate

template<class ControllerType, class CameraType, class ContextType>
void GraphicsSceneContext<ControllerType, CameraType, ContextType>::setupPasses(const std::vector<std::string>& gProgramSignatures,
																				const std::vector<std::string>& lProgramSignatures)
{
	// TODO: might want to manage passes as well
	map<string, ShaderProgramPipeline*> gPrograms;

	for (const auto& programSignature : gProgramSignatures)
	{
		gPrograms[programSignature] = ShaderProgramPipeline::getPipeline(programSignature);
	}

	GeometryPass* gP = new GeometryPass(gPrograms, "GEOMETRYPASS", nullptr, 1);
	gP->setupCamera(cameras[0]);

	map<string, ShaderProgramPipeline*> lPrograms;

	for (const auto& programSignature : lProgramSignatures)
	{
		lPrograms[programSignature] = ShaderProgramPipeline::getPipeline(programSignature);
	}

	LightPass* lP = new LightPass(lPrograms, true);

	gP->addNeighbor(lP);

	passRootNode = gP;
}

template<class ControllerType, class CameraType, class ContextType>
void GraphicsSceneContext<ControllerType, CameraType, ContextType>::makeQuad(void)
{
	// TODO : not a big deal, but primitive vertices should only exist once on the GPU per program and transformed accordingly using Model matrix or instancing
	auto displayQuad = new Quad();

	vector<vec2> uvMap;
	uvMap.push_back(vec2(0, 0));
	uvMap.push_back(vec2(1, 0));
	uvMap.push_back(vec2(0, 1));
	uvMap.push_back(vec2(1, 1));

	auto displayQuadUV = new Graphics::ExtendedMeshObject<vec2, float>(displayQuad, uvMap, "TEXTURECOORD");

	geometries["DISPLAYQUAD"] = displayQuadUV;
}

template<class ControllerType, class CameraType, class ContextType>
void GraphicsSceneContext<ControllerType, CameraType, ContextType>::update(void)
{
	if (dirty)
	{
		if (passRootNode != nullptr)
		{
			passRootNode->execute();
		}
		dirty = false;
	}
}

#pragma endregion
