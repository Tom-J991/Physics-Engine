#pragma once

#include "PhysicsObject.h"

class RigidBody : public PhysicsObject // RigidBody physics object, implements classical mechanics (Newton's laws).
{
public:
	RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity);
	virtual ~RigidBody();

	virtual void FixedUpdate(float timeStep) override;
	virtual void ResetPosition() override;
	
	void ApplyForce(glm::vec2 force, glm::vec2 pos);

	void ResolveCollision(RigidBody *actor2, glm::vec2 contact, glm::vec2 *collisionNormal = nullptr, float penetration = 0);

	glm::vec2 ToWorld(glm::vec2 localPosition);

	virtual float GetEnergy() override;
	float GetKineticEnergy();
	float GetPotentialEnergy();

	glm::vec2 GetPosition() const { return m_position; }
	glm::vec2 GetVelocity() const { return m_velocity; }
	float GetMass() const { return m_kinematic ? INT_MAX : m_mass; }
	float GetOrientation() const { return m_orientation; }
	float GetAngularVelocity() const { return m_angularVelocity; }
	float GetLinearDrag() const { return m_linearDrag; }
	float GetAngularDrag() const { return m_angularDrag; }
	float GetMoment() const { return m_kinematic ? INT_MAX : m_moment; }

	glm::vec2 GetLocalX() const { return m_localX; }
	glm::vec2 GetLocalY() const { return m_localY; }

	bool IsKinematic() const { return m_kinematic; }

	void SetPosition(const glm::vec2 position) { m_position = position; }
	void SetVelocity(const glm::vec2 velocity) { m_velocity = velocity; }
	void SetKinematic(const bool kinematic) { m_kinematic = kinematic; }

protected:
	glm::vec2 m_position;
	glm::vec2 m_velocity;

	float m_mass;
	float m_orientation; // Rotation (radians).
	float m_angularVelocity;
	float m_linearDrag;
	float m_angularDrag;
	float m_moment; // Moment of inertia.

	glm::vec2 m_localX; // Hold two rotation matrix axes for performance reasons.
	glm::vec2 m_localY;

	bool m_kinematic = false;

};
