#include "Sphere.h"

#include "Gizmos.h"

Sphere::Sphere(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, glm::vec4 colour)
	: RigidBody(ShapeType::SPHERE, position, velocity, mass, orientation, angularVelocity, linearDrag, angularDrag, elasticity)
	, m_radius(radius)
	, m_colour(colour)
{ 
	m_moment = 0.5f * m_mass * m_radius * m_radius;
}
Sphere::~Sphere()
{ }

void Sphere::Draw()
{
	glm::vec2 end = glm::vec2(std::cos(m_orientation), std::sin(m_orientation)) * m_radius;
	glm::vec4 outlineColour = m_colour - 0.333f;
	outlineColour.a = 1.0f;
	aie::Gizmos::add2DCircle(m_position, m_radius, 24, outlineColour);
	aie::Gizmos::add2DCircle(m_position, m_radius - 1, 12, m_colour);
	//aie::Gizmos::add2DLine(m_position, m_position + end, { 1, 1, 1, 1 }); // Rotation line. (except rotation doesn't work)
}

bool Sphere::IsInside(glm::vec2 point)
{
	return glm::distance(point, m_position) <= m_radius;
}
