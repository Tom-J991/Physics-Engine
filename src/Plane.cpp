#include "Plane.h"

#include "Gizmos.h"

#include "RigidBody.h"

Plane::Plane(glm::vec2 normal, float distance, glm::vec4 colour)
	: PhysicsObject(ShapeType::PLANE)
	, m_distanceToOrigin(distance)
	, m_colour(colour)
{ 
	m_normal = glm::normalize(normal);
}
Plane::~Plane()
{ }

void Plane::FixedUpdate(float timeStep)
{

}

void Plane::Draw()
{
	// Draws the (infinite) line as a quad.
	float lineSegmentLength = 256; // Limit length since you can't draw infinitely.
	float lineDepth = 12.0f; // The depth/height of the quad.

	glm::vec2 centerPoint = m_normal * m_distanceToOrigin;
	glm::vec2 parallel(m_normal.y, -m_normal.x);

	glm::vec4 colourFade = m_colour - 0.5f; // Gradient
	colourFade.a = 0.0f;

	glm::vec2 start = centerPoint + (parallel * lineSegmentLength); // Leftmost vertex of quad.
	glm::vec2 end = centerPoint - (parallel * lineSegmentLength); // Rightmost vertex of quad.

	//aie::Gizmos::add2DLine(start, end, m_colour); // Draw the line.

	aie::Gizmos::add2DTri(start, end, start - m_normal * lineDepth, m_colour, m_colour, colourFade); // Draw first triangle of quad
	aie::Gizmos::add2DTri(end, end - m_normal * lineDepth, start - m_normal * lineDepth, m_colour, colourFade, colourFade); // Draw second triangle of quad
}

void Plane::ResetPosition()
{
	m_normal = glm::normalize(glm::vec2(0, 1));
	m_distanceToOrigin = 0.0f;
}

void Plane::ResolveCollision(RigidBody *actor2) // Calculates the impulse magnitude and impulse force against a RigidBody object and the static plane and applies it to the RigidBody.
{
	glm::vec2 relVelocity = actor2->GetVelocity();

	float elasticity = 1.0f;
	float impulseMag =	glm::dot(-(1 + elasticity) * (relVelocity), m_normal) / (1 / actor2->GetMass());  // j = (-(1+e)Vrel)*n / (1/Ma)
	glm::vec2 force = m_normal * impulseMag;

	actor2->ApplyForce(force);
}
