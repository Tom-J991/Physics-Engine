#include "Sphere.h"

#include "Gizmos.h"

Sphere::Sphere(glm::vec2 position, glm::vec2 velocity, float mass, float radius, glm::vec4 colour)
	: RigidBody(ShapeType::SPHERE, position, velocity, 0, mass)
	, m_radius(radius)
	, m_colour(colour)
{ }
Sphere::~Sphere()
{ }

void Sphere::Draw()
{
	glm::vec4 outlineColour = m_colour - 0.333f;
	outlineColour.a = 1.0f;
	aie::Gizmos::add2DCircle(m_position, m_radius, 24, outlineColour);
	aie::Gizmos::add2DCircle(m_position, m_radius-1, 12, m_colour);
}
