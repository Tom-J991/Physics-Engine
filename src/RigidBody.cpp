#include "RigidBody.h"

RigidBody::RigidBody(ShapeType shapeID, glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity)
	: PhysicsObject(shapeID)
	, m_position(position)
	, m_velocity(velocity)
	, m_mass(mass)
	, m_orientation(orientation)
	, m_angularVelocity(angularVelocity)
	, m_moment(1.0f)
{ }
RigidBody::~RigidBody()
{ }

void RigidBody::FixedUpdate(float timeStep)
{
	m_position += m_velocity * timeStep; // Apply velocity to position.
	if (m_kinematic == false)
		ApplyForce(m_physicsScene->GetGravity() * m_mass * timeStep, { 0.0f, 0.0f }); // Do gravity.

	m_orientation += m_angularVelocity * timeStep; // Apply angular velocity to rotation.
}

void RigidBody::ResetPosition()
{
}

void RigidBody::ApplyForce(glm::vec2 force, glm::vec2 pos)
{
	m_velocity += (force / GetMass()); // force = mass * acceleration, acceleration = force / mass. Newton's second law.
	m_angularVelocity += (force.y * pos.x - force.x * pos.y) / GetMoment(); // angular acceleration = torque / moment of inertia. torque = force applied * lever arm (t=||r||||f||sin(angle)).
}

#include <iostream>
void RigidBody::ResolveCollision(RigidBody *actor2, glm::vec2 contact, glm::vec2 *collisionNormal) 
// Calculates effective mass from the collision normal, and the velocity/angular velocity of each actor at the contact point then applies the corrective force (equal and opposite) to the actors if the contact points are moving closer.
{
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
		float mass1 = 1.0f / (1.0f / m_mass + (r1 * r1) / m_moment); // Calculate effective mass at the point of contact for both objects.
		float mass2 = 1.0f / (1.0f / actor2->GetMass() + (r2 * r2) / actor2->GetMoment()); // effective mass = 1 / (1 / mass) + (radius^2 / moment of inertia)

		float elasticity = 1.0f;
		float impulseMag =	glm::dot(-(1 + elasticity) * (relVelocity), normal) /
							glm::dot(normal, normal * ((1 / mass1) + (1 / mass2))); // j = (-(1+e)Vrel)*n / n*(n*(1/Ma + 1/Mb))
		glm::vec2 force = normal * impulseMag;

		float initialKineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

		// Equal and opposite reactions.
		if (m_kinematic == false)
			ApplyForce(-force, contact - m_position);
		actor2->ApplyForce(force, contact - actor2->GetPosition());

		float kineticEnergy = GetKineticEnergy() + actor2->GetKineticEnergy(); // Diagnostics.

		float deltaKineticEnergy = kineticEnergy - initialKineticEnergy;
		if (deltaKineticEnergy > kineticEnergy * 0.01f)
			std::cout << "Kinetic Energy discrepancy greater than 1%";
	}
}

float RigidBody::GetEnergy()
{
	return GetKineticEnergy() + GetPotentialEnergy();
}

float RigidBody::GetKineticEnergy()
{
	return 0.5f * (m_mass * glm::dot(m_velocity, m_velocity) + m_moment * (m_angularVelocity * m_angularVelocity)); // E = 0.5(mv^2), E = 0.5(IӨ^2)
}

float RigidBody::GetPotentialEnergy()
{
	return -m_mass * glm::dot(m_physicsScene->GetGravity(), m_position); // E = -mgh
}
