#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include "PhysicsScene.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Application2D : public aie::Application 
{
public:
	Application2D();
	virtual ~Application2D();

	virtual bool startup() override;
	virtual void shutdown() override;

	virtual void update(float deltaTime) override;
	virtual void draw() override;

	glm::vec2 ScreenToWorld(glm::vec2 screenPosition);

protected:
	aie::Renderer2D *m_2dRenderer = nullptr;

	PhysicsScene *m_physicsScene = nullptr;

};
