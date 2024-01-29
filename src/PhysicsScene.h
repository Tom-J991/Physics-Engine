#pragma once

#include "Renderer2D.h"

#include <vector>

#include <glm/glm.hpp>

class PhysicsObject;

class PhysicsScene
{
public:
	PhysicsScene();
	~PhysicsScene();

	void AddActor(PhysicsObject *actor);
	void RemoveActor(PhysicsObject *actor);

	void Update(float deltaTime);
	void Draw();

	void SetGravity(const glm::vec2 gravity) { m_gravity = gravity; }
	glm::vec2 GetGravity() const { return m_gravity; }

	void SetTimeStep(const float timeStep) { m_timeStep = timeStep; }
	float GetTimeStep() const { return m_timeStep; }

protected:
	float m_timeStep;
	glm::vec2 m_gravity;

	std::vector<PhysicsObject *> m_actors;

};
