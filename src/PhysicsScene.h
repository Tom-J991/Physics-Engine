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

	float GetTotalEnergy();

	// Collision between different objects.
	static bool Plane2Plane(PhysicsObject *, PhysicsObject *);
	static bool Plane2Sphere(PhysicsObject *, PhysicsObject *);
	static bool Plane2Box(PhysicsObject *, PhysicsObject *);
	static bool Sphere2Plane(PhysicsObject *, PhysicsObject *);
	static bool Sphere2Sphere(PhysicsObject *, PhysicsObject *);
	static bool Sphere2Box(PhysicsObject *, PhysicsObject *);
	static bool Box2Plane(PhysicsObject *, PhysicsObject *);
	static bool Box2Sphere(PhysicsObject *, PhysicsObject *);
	static bool Box2Box(PhysicsObject *, PhysicsObject *);

	void SetGravity(const glm::vec2 gravity) { m_gravity = gravity; }
	glm::vec2 GetGravity() const { return m_gravity; }

	void SetTimeStep(const float timeStep) { m_timeStep = timeStep; }
	float GetTimeStep() const { return m_timeStep; }

protected:
	glm::vec2 m_gravity; // Global gravity.
	float m_timeStep;

	std::vector<PhysicsObject *> m_actors;

};
