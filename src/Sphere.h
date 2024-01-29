#pragma once

#include "RigidBody.h"

#include "Renderer2D.h"

#include <glm/glm.hpp>

class Sphere : public RigidBody // Sphere RigidBody object, simply derives from the RigidBody class and draws a sphere in position.
{
public:
	Sphere(glm::vec2 position, glm::vec2 velocity, float mass, float radius, glm::vec4 colour);
	~Sphere();

	virtual void Draw();

	float GetRadius() { return m_radius; }
	glm::vec4 GetColour() { return m_colour; }

protected:
	float m_radius;
	glm::vec4 m_colour;

};
