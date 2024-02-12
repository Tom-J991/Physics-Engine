#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include "PhysicsScene.h"

#include <fmod.h>

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

	static glm::vec2 ScreenToWorld(glm::vec2 screenPosition);

private:
	void BallInHole(PhysicsObject *collisionObj, PhysicsObject *other); // Callback function.

	void BallCollided(PhysicsObject *collisionObj, PhysicsObject *other);

protected:
	aie::Renderer2D *m_2dRenderer = nullptr;

	PhysicsScene *m_physicsScene = nullptr;
	
	FMOD_SYSTEM *m_fmodSystem = nullptr;

};
