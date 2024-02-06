#include "RigidBody.h"

#define MIN_LINEAR_THRESHOLD 0.1f
#define MIN_ANGULAR_THRESHOLD 0.01f

RigidBody::RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity)
	: PhysicsObject(shapeID)
	, m_position(position)
	, m_velocity(velocity)
	, m_mass(mass)
	, m_orientation(orientation)
	, m_angularVelocity(angularVelocity)
	, m_linearDrag(linearDrag)
	, m_angularDrag(angularDrag)
{ 
	m_moment = 1.0f;
	m_elasticity = elasticity;
}
RigidBody::~RigidBody()
{ }

void RigidBody::FixedUpdate(float timeStep)
{
	float cs = glm::cos(m_orientation);
	float sn = glm::sin(m_orientation);
	m_localX = glm::normalize(glm::vec2(cs, sn));
	m_localY = glm::normalize(glm::vec2(-sn, cs));

	if (m_isTrigger)
	{
		for (std::list<PhysicsObject *>::iterator it = m_objectsInside.begin(); it != m_objectsInside.end(); ++it)
		{
			if (std::find(m_objectsInsideThisFrame.begin(), m_objectsInsideThisFrame.end(), *it) == m_objectsInsideThisFrame.end())
			{
				if (triggerExit)
					triggerExit(*it);
				it = m_objectsInside.erase(it);
				if (it == m_objectsInside.end())
					break;
			}
		}
	}
	m_objectsInsideThisFrame.clear();

	if (m_kinematic == true)
	{
		m_velocity = { 0.0f, 0.0f };
		m_angularVelocity = 0.0f;
		return;
	}

	m_position += m_velocity * timeStep; // Apply velocity to position.
	ApplyForce(m_physicsScene->GetGravity() * GetMass() * timeStep, { 0.0f, 0.0f }); // Do gravity.
	m_velocity -= m_velocity * m_linearDrag * timeStep; // Apply linear drag to velocity.

	if (m_lockRotation == false)
	{
		m_orientation += m_angularVelocity * timeStep; // Apply angular velocity to rotation.
		m_angularVelocity -= m_angularVelocity * m_angularDrag * timeStep; // Apply angular drag to angular velocity.
	}

	if (glm::length(m_velocity) < MIN_LINEAR_THRESHOLD)
	{
		m_velocity = { 0.0f, 0.0f }; // Stop rigidbody from moving after enough drag.
	}
	if (abs(m_angularVelocity) < MIN_ANGULAR_THRESHOLD)
	{
		m_angularVelocity = 0.0f; // Stop rigidbody from rotating after enough angular drag.
	}
}

void RigidBody::ResetPosition()
{
}

void RigidBody::TriggerEnter(PhysicsObject *actor2)
{
	if (m_isTrigger && std::find(m_objectsInside.begin(), m_objectsInside.end(), actor2) == m_objectsInside.end())
	{
		m_objectsInside.push_back(actor2);
		if (triggerEnter != nullptr)
			triggerEnter(actor2);
	}
}

void RigidBody::ApplyForce(glm::vec2 force, glm::vec2 pos)
{
	m_velocity += (force / GetMass()); // force = mass * acceleration, acceleration = force / mass. Newton's second law.
	if (m_lockRotation == false) m_angularVelocity += (force.y * pos.x - force.x * pos.y) / GetMoment(); // angular acceleration = torque / moment of inertia. torque = force applied * lever arm (t=||r||||f||sin(angle)).
}

#include <iostream>
void RigidBody::ResolveCollision(RigidBody *actor2, glm::vec2 contact, glm::vec2 *collisionNormal, float penetration)
// Calculates effective mass from the collision normal, and the velocity/angular velocity of each actor at the contact point then applies the corrective force (equal and opposite) to the actors if the contact points are moving closer.
{
	m_objectsInsideThisFrame.push_back(actor2);
	actor2->m_objectsInsideThisFrame.push_back(this);

	glm::vec2 normal = glm::normalize(collisionNormal ? *collisionNormal : actor2->GetPosition() - m_position);
	glm::vec2 relVelocity = actor2->GetVelocity() - m_velocity;

	glm::vec2 perpendicular = glm::vec2(normal.y, -normal.x);

	// 'r' = Radius from axis to application of force.
	float r1 = glm::dot(contact - m_position, -perpendicular);
	float r2 = glm::dot(contact - actor2->GetPosition(), perpendicular);

	float v1 = glm::dot(m_velocity, normal) - r1 * m_angularVelocity; // Velocity of the contact point.
	float v2 = glm::dot(actor2->GetVelocity(), normal) + r2 * actor2->GetAngularVelocity(); // Velocity of the contact point on actor2.
	if (v1 > v2) // Are they getting closer to each other?
	{
		float mass1 = 1.0f / (1.0f / GetMass() + (r1 * r1) / GetMoment()); // Calculate effective mass at the point of contact for both objects.
		float mass2 = 1.0f / (1.0f / actor2->GetMass() + (r2 * r2) / actor2->GetMoment()); // effective mass = 1 / (1 / mass) + (radius^2 / moment of inertia)

		float elasticity = (m_elasticity + actor2->GetElasticity()) / 2.0f;
		float impulseMag =	glm::dot(-(1 + elasticity) * (relVelocity), normal) /
							glm::dot(normal, normal * ((1 / mass1) + (1 / mass2))); // j = (-(1+e)Vrel)*n / n*(n*(1/Ma + 1/Mb))
		glm::vec2 force = normal * impulseMag;

		float initialKineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

		// Equal and opposite reactions.
		if (!m_isTrigger && !actor2->m_isTrigger)
		{
			if (m_kinematic == false)
				ApplyForce(-force, contact - m_position);
			if (actor2->IsKinematic() == false)
				actor2->ApplyForce(force, contact - actor2->GetPosition());

			if (collisionCallback != nullptr)
				collisionCallback(actor2);
			if (actor2->collisionCallback != nullptr)
				actor2->collisionCallback(this);
		}
		else
		{
			TriggerEnter(actor2);
			actor2->TriggerEnter(this);
		}

		float kineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

		float deltaKineticEnergy = kineticEnergy - initialKineticEnergy;
		if (deltaKineticEnergy > kineticEnergy * 0.01f)
			std::cout << "Kinetic Energy discrepancy greater than 1%";
	}

	if (penetration > 0)
	{
		PhysicsScene::ApplyContactForces(this, actor2, normal, penetration);
	}
}

glm::vec2 RigidBody::ToWorld(glm::vec2 localPosition)
{
	glm::vec2 worldX = glm::vec2(1, 0);
	glm::vec2 worldY = glm::vec2(0, 1);
	return localPosition + worldX * m_position.x + worldY * m_position.y;
}

float RigidBody::GetEnergy()
{
	return GetKineticEnergy() + GetPotentialEnergy();
}

float RigidBody::GetKineticEnergy()
{
	return 0.5f * (GetMass() * glm::dot(m_velocity, m_velocity) + GetMoment() * (m_angularVelocity * m_angularVelocity)); // E = 0.5(mv^2), E = 0.5(IӨ^2)
}

float RigidBody::GetPotentialEnergy()
{
	return -GetMass() * glm::dot(m_physicsScene->GetGravity(), m_position); // E = -mgh
}
