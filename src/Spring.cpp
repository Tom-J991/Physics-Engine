#include "Spring.h"

#include "Gizmos.h"

Spring::Spring(RigidBody *body1, RigidBody *body2, float damping, float restLength, float springCoefficient, glm::vec2 contact1, glm::vec2 contact2)
	: RigidBody(JOINT, { 0, 0 }, { 0, 0 }, (float)INT_MAX, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)
	, m_body1(body1)
	, m_body2(body2)
	, m_contact1(contact1)
	, m_contact2(contact2)
	, m_damping(damping)
	, m_restLength(restLength)
	, m_springCoefficient(springCoefficient)
{ }
Spring::~Spring()
{ 
	RigidBody::~RigidBody();
}

void Spring::FixedUpdate(float timeStep)
{
	if (m_body1 == nullptr && m_body2 == nullptr)
		return;

	glm::vec2 relativeVelocity;

	glm::vec2 p1 = GetFirstContact();
	glm::vec2 p2 = GetSecondContact();

	float length = glm::distance(p1, p2);
	glm::vec2 direction = glm::normalize(p2 - p1);

	if (m_body1 == nullptr)
		relativeVelocity = m_body2->GetVelocity();
	else if (m_body2 == nullptr)
		relativeVelocity = m_body1->GetVelocity();
	else
		relativeVelocity = m_body2->GetVelocity() - m_body1->GetVelocity();

	glm::vec2 force = direction * m_springCoefficient * (m_restLength - length) - m_damping * relativeVelocity; // F = -kX - bv

	if (m_body1 != nullptr)
		m_body1->ApplyForce(-force * timeStep, p1 - m_body1->GetPosition());
	if (m_body2 != nullptr)
		m_body2->ApplyForce(force * timeStep, p2 - m_body2->GetPosition());
}

void Spring::Draw()
{
	aie::Gizmos::add2DLine(GetFirstContact(), GetSecondContact(), { 1, 1, 0, 1 }); // Yellow.
}

void Spring::ResetPosition()
{
}
