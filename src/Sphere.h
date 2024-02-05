#pragma once

#include "RigidBody.h"

#include "Renderer2D.h"

class Sphere : public RigidBody // Sphere RigidBody object, simply derives from the RigidBody class and draws a sphere in position.
{
public:
	Sphere(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, glm::vec4 colour = { 1, 0, 0, 1 });
	virtual ~Sphere();

	virtual void Draw() override;

	float GetRadius() const { return m_radius; }
	glm::vec4 GetColour() const { return m_colour; }

	void SetColour(const glm::vec4 colour) { m_colour = colour; }

protected:
	float m_radius;
	glm::vec4 m_colour;

};
