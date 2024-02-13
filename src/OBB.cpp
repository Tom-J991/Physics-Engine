#include "OBB.h"

#include "Common.h"

#include "Gizmos.h"

OBB::OBB(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, glm::vec2 extents, glm::vec4 colour)
	: RigidBody(ShapeType::BOX, position, velocity, mass, orientation, angularVelocity, linearDrag, angularDrag, elasticity)
	, m_extents(extents)
	, m_colour(colour)
{ 
	m_moment = 1.0f / 12.0f * m_mass * GetWidth() * GetHeight(); // Moment of inertia for box.
}
OBB::~OBB()
{ 
	RigidBody::~RigidBody();
}

void OBB::Draw()
{
	glm::vec4 drawColor = m_colour;
	drawColor.a = GLOBALS::g_DEBUG ? 1.0f : m_colour.a;

	if (drawColor.a <= 0) // Don't need to draw if it's gonna be invisible anyways.
		return;

	glm::vec2 p1 = m_position - GetLocalX() * m_extents.x - GetLocalY() * m_extents.y; // Find vertices of box.
	glm::vec2 p2 = m_position + GetLocalX() * m_extents.x - GetLocalY() * m_extents.y;
	glm::vec2 p3 = m_position - GetLocalX() * m_extents.x + GetLocalY() * m_extents.y;
	glm::vec2 p4 = m_position + GetLocalX() * m_extents.x + GetLocalY() * m_extents.y;
	aie::Gizmos::add2DTri(p1, p2, p4, drawColor); // BUG: For some reason these gizmos can just randomly stop drawing, collision/physics still works.
	aie::Gizmos::add2DTri(p1, p4, p3, drawColor);
}

bool OBB::CheckBoxCorners(const OBB &box, glm::vec2 &contact, int &numContacts, float &pen, glm::vec2 &edgeNormal)
{
	// Check if the other box's corners are intersecting this.
	float minX, maxX, minY, maxY;
	float boxW = GetWidth();
	float boxH = GetHeight();
	int numLocalContacts = 0;
	glm::vec2 localContact = { 0, 0 };
	bool first = true;

	// Iterate through each corner of the other box.
	for (float x = -box.GetExtents().x; x < boxW; x += boxW)
	{
		for (float y = -box.GetExtents().y; y < boxH; y += boxH)
		{
			// Get the position in world space.
			glm::vec2 p = box.GetPosition() + x * box.GetLocalX() + y * box.GetLocalY();
			glm::vec2 p0(glm::dot(p - m_position, GetLocalX()), glm::dot(p - m_position, GetLocalY())); // The position in this box's coordinate space.

			// Update the extents along the separating axies.
			if (first || p0.x < minX) minX = p0.x;
			if (first || p0.x > maxX) maxX = p0.x;
			if (first || p0.y < minY) minY = p0.y;
			if (first || p0.y > maxY) maxY = p0.y;

			// If the corner is inside the box then count it.
			if (p0.x >= -m_extents.y && p0.x <= m_extents.x && p0.y >= -m_extents.y && p0.y <= m_extents.y)
			{
				numLocalContacts++;
				localContact += p0;
			}
			first = false;
		}
	}

	// A separating axis is found.
	if (maxX <= -m_extents.x || minX >= m_extents.x || maxY <= -m_extents.y || minY >= m_extents.y)
		return false;
	if (numLocalContacts == 0) // No contacts found.
		return false;

	bool res = false;
	contact += m_position + (localContact.x * GetLocalX() + localContact.y * GetLocalY()) / (float)numLocalContacts;
	numContacts++;

	// Find the minimum penetration vector.
	float pen0 = m_extents.x - minX;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = GetLocalX();
		pen = pen0;
		res = true;
	}
	pen0 = maxX + m_extents.x;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = -GetLocalX();
		pen = pen0;
		res = true;
	}
	pen0 = m_extents.y - minY;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = GetLocalY();
		pen = pen0;
		res = true;
	}
	pen0 = maxY + m_extents.y;
	if (pen0 > 0 && (pen0 < pen || pen == 0))
	{
		edgeNormal = -GetLocalY();
		pen = pen0;
		res = true;
	}

	return res;
}
