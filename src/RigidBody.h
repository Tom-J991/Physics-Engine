#pragma once

#include "PhysicsObject.h"

#include <glm/glm.hpp>

class RigidBody : public PhysicsObject // RigidBody physics object, implements classical mechanics (Newton's laws).
{
public:
	RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float orientation, float mass);
	~RigidBody();

	virtual void FixedUpdate(glm::vec2 gravity, float timeStep);
	
	void ApplyForce(glm::vec2 force);
	void ApplyForceToActor(RigidBody *actor2, glm::vec2 force); // Applies force against another rigidbody object.

	glm::vec2 GetPosition() { return m_position; }
	glm::vec2 GetVelocity() { return m_velocity; }
	float GetMass() { return m_mass; }
	float GetOrientation() { return m_orientation; }

	void SetVelocity(const glm::vec2 velocity) { m_velocity = velocity; }

protected:
	glm::vec2 m_position;
	glm::vec2 m_velocity;
	float m_mass;
	float m_orientation;

};
