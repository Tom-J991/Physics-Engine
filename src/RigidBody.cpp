#include "RigidBody.h"

RigidBody::RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float orientation, float mass)
	: PhysicsObject(shapeID)
	, m_position(position)
	, m_velocity(velocity)
	, m_mass(mass)
	, m_orientation(orientation)
{ }
RigidBody::~RigidBody()
{ }

void RigidBody::FixedUpdate(float timeStep)
{
	ApplyForce(m_physicsScene->GetGravity() * m_mass * timeStep); // Do gravity.
	m_position += m_velocity * timeStep; // Apply velocity.
}

void RigidBody::ApplyForce(glm::vec2 force)
{
	m_velocity += (force / m_mass); // force = mass * acceleration, acceleration = force / mass. Newton's second law.
}

void RigidBody::ApplyForceToActor(RigidBody *actor2, glm::vec2 force)
{
	actor2->ApplyForce(force); // Apply force against other rigidbody and opposite force against self. Newton's third law.
	ApplyForce(-force);
}

#include <iostream>
void RigidBody::ResolveCollision(RigidBody *actor2) // Calculates impulse magnitude and impulse force from the actor's velocity and the collision normal then applies the equal and opposite reactions to the actors.
{
	glm::vec2 normal = glm::normalize(actor2->GetPosition() - m_position);
	glm::vec2 relVelocity = actor2->GetVelocity() - m_velocity;

	float elasticity = 1.0f;
	float impulseMag =	glm::dot(-(1 + elasticity) * (relVelocity), normal) / 
						glm::dot(normal, normal *((1 / m_mass) + (1 / actor2->GetMass()))); // j = (-(1+e)Vrel)*n / n*(n*(1/Ma + 1/Mb))
	glm::vec2 force = normal * impulseMag;

	float initialKineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

	ApplyForceToActor(actor2, force);

	float kineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

	float deltaKineticEnergy = kineticEnergy - initialKineticEnergy;
	if (deltaKineticEnergy > kineticEnergy * 0.01f)
		std::cout << "Kinetic Energy discrepancy greater than 1%";
}

float RigidBody::GetEnergy()
{
	return GetKineticEnergy() + GetPotentialEnergy();
}

float RigidBody::GetKineticEnergy()
{
	return 0.5f * (m_mass * glm::dot(m_velocity, m_velocity)); // E = 0.5(mv^2)
}

float RigidBody::GetPotentialEnergy()
{
	return -m_mass * glm::dot(m_physicsScene->GetGravity(), m_position); // E = -mgh
}
