#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include "PhysicsScene.h"

class Application2D : public aie::Application 
{
public:
	Application2D();
	virtual ~Application2D();

	virtual bool startup() override;
	virtual void shutdown() override;

	virtual void update(float deltaTime) override;
	virtual void draw() override;

protected:
	aie::Renderer2D *m_2dRenderer = nullptr;

	PhysicsScene *m_physicsScene = nullptr;

};
