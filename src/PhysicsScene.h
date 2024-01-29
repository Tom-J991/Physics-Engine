#pragma once

#include "Renderer2D.h"

#include <vector>

#include <glm/glm.hpp>

class PhysicsObject;

class PhysicsScene // Holds a list of all physics objects and updates them on a fixed timestep, though draws them each frame.
{
public:
	PhysicsScene();
	~PhysicsScene();

	void AddActor(PhysicsObject *actor); // Adds and removes physics objects from the list.
	void RemoveActor(PhysicsObject *actor);

	void Update(float deltaTime);
	void Draw();

	static bool Sphere2Sphere(PhysicsObject *obj1, PhysicsObject *obj2); // Sphere-Sphere collision stuff.

	void SetGravity(const glm::vec2 gravity) { m_gravity = gravity; }
	glm::vec2 GetGravity() const { return m_gravity; }

	void SetTimeStep(const float timeStep) { m_timeStep = timeStep; }
	float GetTimeStep() const { return m_timeStep; }

protected:
	float m_timeStep;
	glm::vec2 m_gravity; // Global gravity.

	std::vector<PhysicsObject *> m_actors;

};
