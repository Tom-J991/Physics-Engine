#pragma once

#include "Renderer2D.h"

#include "PhysicsObject.h"

class RigidBody;

class Plane : public PhysicsObject
{
public:
	Plane(glm::vec2 normal = { 0.f, 1.f }, float distance = 0.0f, glm::vec4 colour = { 0, 1, 0, 1 });
	virtual ~Plane();

	virtual void FixedUpdate(float timeStep) override;
	virtual void Draw() override;
	virtual void ResetPosition() override;

	void ResolveCollision(RigidBody *actor2);

	glm::vec2 GetNormal() { return m_normal; }
	float GetDistance() { return m_distanceToOrigin; }

protected:
	glm::vec2 m_normal;
	float m_distanceToOrigin;

	glm::vec4 m_colour;

};
