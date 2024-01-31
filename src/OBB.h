#pragma once

#include "RigidBody.h"

#include "Renderer2D.h"

class OBB : public RigidBody
{
public:
	OBB(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, glm::vec2 extents, glm::vec4 colour = { 1, 1, 0, 1 });
	virtual ~OBB();

	virtual void FixedUpdate(float timeStep) override;
	virtual void Draw() override;

	bool CheckBoxCorners(const OBB &box, glm::vec2 &contact, int &numContacts, float &pen, glm::vec2 &edgeNormal);

	glm::vec2 GetExtents() const { return m_extents; }
	glm::vec4 GetColour() const { return m_colour; }

	float GetWidth() const { return m_extents.x * 2; }
	float GetHeight() const { return m_extents.y * 2; }

	glm::vec2 GetLocalX() const { return m_localX; }
	glm::vec2 GetLocalY() const { return m_localY; }

	void SetColour(const glm::vec4 colour) { m_colour = colour; }

protected:
	glm::vec2 m_extents;
	glm::vec4 m_colour;

	glm::vec2 m_localX; // Hold two rotation matrix axes for performance reasons.
	glm::vec2 m_localY;

};
