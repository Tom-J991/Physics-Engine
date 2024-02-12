#include "Plane.h"

#include "Common.h"

#include "Gizmos.h"

#include "RigidBody.h"

Plane::Plane(float elasticity, glm::vec2 normal, float distance, glm::vec4 colour, float length, float depth)
	: PhysicsObject(ShapeType::PLANE)
	, m_distanceToOrigin(distance)
	, m_colour(colour)
	, m_length(length)
	, m_depth(depth)
{ 
	m_elasticity = elasticity;
	m_normal = glm::normalize(normal);
}
Plane::~Plane()
{ }

void Plane::FixedUpdate(float timeStep)
{

}

void Plane::Draw()
{
	glm::vec4 drawColor = m_colour;
	drawColor.a = GLOBALS::g_DEBUG ? 1.0f : m_colour.a;

	if (drawColor.a <= 0)
		return;

	// Draws the (infinite) line as a quad.
	float lineDepth = m_depth; // The depth/height of the quad.
	float lineSegmentLength = m_length + lineDepth; // Limit length since you can't draw infinitely.

	glm::vec2 centerPoint = m_normal * m_distanceToOrigin;
	glm::vec2 parallel(m_normal.y, -m_normal.x);

	glm::vec4 colourFade = drawColor; // Gradient
	colourFade.a = 0.0f;

	glm::vec2 start = centerPoint + (parallel * lineSegmentLength); // Leftmost vertex of quad.
	glm::vec2 end = centerPoint - (parallel * lineSegmentLength); // Rightmost vertex of quad.

	aie::Gizmos::add2DLine(start, end, drawColor); // Draw the line.

	//aie::Gizmos::add2DTri(start, end, start - m_normal * lineDepth, m_colour, m_colour, colourFade); // Draw first triangle of quad
	//aie::Gizmos::add2DTri(end, end - m_normal * lineDepth, start - m_normal * lineDepth, m_colour, colourFade, colourFade); // Draw second triangle of quad
}

void Plane::ResetPosition()
{
	m_normal = glm::normalize(glm::vec2(0, 1));
	m_distanceToOrigin = 0.0f;
}

#include <iostream>
void Plane::ResolveCollision(RigidBody *actor2, glm::vec2 contact) // Calculates the impulse magnitude and impulse force against a RigidBody object and the static plane and applies it to the RigidBody at the point of contact.
{
	if (actor2->IsKinematic())
		return;

	glm::vec2 localContact = contact - actor2->GetPosition();

	glm::vec2 relVelocity = actor2->GetVelocity() + actor2->GetAngularVelocity() * glm::vec2(-localContact.y, localContact.x);
	float velocityIntoPlane = glm::dot(relVelocity, m_normal);

	float r = glm::dot(localContact, glm::vec2(m_normal.y, -m_normal.x));
	float mass0 = 1.0f / (1.0f / actor2->GetMass() + (r * r) / actor2->GetMoment());

	float elasticity = (m_elasticity + actor2->GetElasticity()) / 2.0f;
	float impulseMag =	-(1 + elasticity) * velocityIntoPlane * mass0;  // j = (-(1+e)Vrel)*n / (1/Ma)
	glm::vec2 force = m_normal * impulseMag;

	float initialKineticEnergy = actor2->GetKineticEnergy(); // Diagnostics.

	actor2->ApplyForce(force, contact - actor2->GetPosition());

	float penetration = glm::dot(contact, m_normal) - m_distanceToOrigin;
	PhysicsScene::ApplyContactForces(actor2, nullptr, m_normal, penetration);

	float kineticEnergy = actor2->GetKineticEnergy(); // Diagnostics.

	float deltaKineticEnergy = kineticEnergy - initialKineticEnergy;
	if (deltaKineticEnergy > kineticEnergy * 0.01f)
		std::cout << "Kinetic Energy discrepancy greater than 1%" << std::endl;
}
