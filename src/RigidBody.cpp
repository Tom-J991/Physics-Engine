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

void RigidBody::FixedUpdate(glm::vec2 gravity, float timeStep)
{
	m_position += m_velocity * timeStep; // Apply velocity.
	ApplyForce(gravity * m_mass * timeStep); // Do gravity.
}

void RigidBody::ApplyForce(glm::vec2 force)
{
	m_velocity += (force / m_mass); // force = mass * acceleration, acceleration = force / mass. Newton's second law.
}

void RigidBody::ApplyForceToActor(RigidBody *actor2, glm::vec2 force)
{
	actor2->ApplyForce(force); // Apply force against other rigidbody and opposite force against self. Newton's third law.
	ApplyForce(force * -1.f);
}
