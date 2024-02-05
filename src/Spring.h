#pragma once

#include "RigidBody.h"

class Spring : public RigidBody
{
public:
	Spring(RigidBody *body1, RigidBody *body2, float damping, float restLength, float springCoefficient, glm::vec2 contact1 = { 0, 0 }, glm::vec2 contact2 = { 0, 0 });
	virtual ~Spring();

	virtual void FixedUpdate(float timeStep) override;
	virtual void Draw() override;
	virtual void ResetPosition() override;

	RigidBody *GetFirstBody() const { return m_body1; }
	RigidBody *GetSecondBody() const { return m_body2; }

	glm::vec2 GetFirstContact() const { return m_body1 ? m_body1->ToWorld(m_contact1) : m_contact1; }
	glm::vec2 GetSecondContact() const { return m_body2 ? m_body2->ToWorld(m_contact2) : m_contact2; }

	float GetDamping() const { return m_damping; }
	float GetRestingLength() const { return m_restLength; }
	float GetSpringCoefficient() const { return m_springCoefficient; }

protected:
	RigidBody *m_body1;
	RigidBody *m_body2;

	glm::vec2 m_contact1;
	glm::vec2 m_contact2;

	float m_damping;
	float m_restLength;
	float m_springCoefficient;

};
