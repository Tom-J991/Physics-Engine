#pragma once

#include "PhysicsObject.h"

#include <functional>

class RigidBody : public PhysicsObject // RigidBody physics object, implements classical mechanics (Newton's laws).
{
public:
	RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity);
	virtual ~RigidBody();

	virtual void FixedUpdate(float timeStep) override;
	virtual void ResetPosition() override;
	
	void TriggerEnter(PhysicsObject *actor2);

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
	bool IsTrigger() const { return m_isTrigger; }

	void SetPosition(const glm::vec2 position) { m_position = position; }
	void SetVelocity(const glm::vec2 velocity) { m_velocity = velocity; }
	void SetKinematic(const bool kinematic) { m_kinematic = kinematic; }
	void SetRotationLock(const bool lock) { m_lockRotation = lock; }
	void SetIsTrigger(const bool trigger) { m_isTrigger = trigger; }

public:
	std::function<void(PhysicsObject *)> collisionCallback;

	std::function<void(PhysicsObject *)> triggerEnter;
	std::function<void(PhysicsObject *)> triggerExit;

protected:
	std::list<PhysicsObject *> m_objectsInside;
	std::list<PhysicsObject *> m_objectsInsideThisFrame;

	glm::vec2 m_position;
	glm::vec2 m_velocity;

	float m_mass;
	float m_orientation; // Rotation (radians).
	float m_angularVelocity;
	float m_linearDrag;
	float m_angularDrag;
	float m_moment; // Moment of inertia.

	glm::vec2 m_localX = { 1, 0 }; // Hold two rotation matrix axes mostly for performance reasons.
	glm::vec2 m_localY = { 0, 1 };

	bool m_kinematic = false;
	bool m_lockRotation = false;
	bool m_isTrigger = false;

};
