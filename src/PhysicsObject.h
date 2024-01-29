#pragma once

#include <glm/glm.hpp>

enum ShapeType
{
	PLANE = 0,
	SPHERE,
	BOX
};

class PhysicsObject // Abstract class to specify objects within the physics scene.
{
public:
	virtual void FixedUpdate(glm::vec2 gravity, float timeStep) = 0;
	virtual void Draw() = 0;
	virtual void ResetPosition() { };

protected:
	PhysicsObject(ShapeType a_shapeID) : m_shapeID(a_shapeID) { }

protected:
	ShapeType m_shapeID;

};
