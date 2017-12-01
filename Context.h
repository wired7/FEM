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
	virtual void setupPasses(void) = 0;
	virtual void makeQuad(void);
public:
	bool dirty = true;
	Pass* passRootNode;
	vector<Graphics::DecoratedGraphicsObject*> geometries;
	GraphicsSceneContext() {};
	~GraphicsSceneContext() {};
	virtual void update(void);
};

#pragma region GraphicsSceneContextTemplate

template<class ControllerType, class CameraType, class ContextType> void GraphicsSceneContext<ControllerType, CameraType, ContextType>::makeQuad(void)
{
	// TODO : not a big deal, but primitive vertices should only exist once on the GPU per program and transformed accordingly using Model matrix or instancing
	auto displayQuad = new Quad();

	vector<vec2> uvMap;
	uvMap.push_back(vec2(0, 0));
	uvMap.push_back(vec2(1, 0));
	uvMap.push_back(vec2(0, 1));
	uvMap.push_back(vec2(1, 1));

	auto displayQuadUV = new Graphics::ExtendedMeshObject<vec2, float>(displayQuad, uvMap, "TEXTURECOORD");

	geometries.push_back(displayQuadUV);
}

template<class ControllerType, class CameraType, class ContextType> void GraphicsSceneContext<ControllerType, CameraType, ContextType>::update(void)
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
