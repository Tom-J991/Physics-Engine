#pragma once

#include <glm/glm.hpp>

#include "PhysicsScene.h"

enum ShapeType
{
	JOINT = -1,
	PLANE = 0,
	SPHERE,
	BOX,
	SHAPE_COUNT
};

class PhysicsObject // Abstract class to specify objects within the physics scene.
{
public:
	virtual void FixedUpdate(float timeStep) = 0;
	virtual void Draw() = 0;
	virtual void ResetPosition() { };

	virtual float GetEnergy() { return 0.0f; };

	ShapeType GetShapeID() const { return m_shapeID; }
	float GetElasticity() const { return m_elasticity; }

	void SetPhysicsScene(PhysicsScene *physicsScene) { m_physicsScene = physicsScene; }

protected:
	PhysicsObject(ShapeType a_shapeID) : m_shapeID(a_shapeID) { }

protected:
	PhysicsScene *m_physicsScene = nullptr; // Keep a pointer to the parent physics scene.
	ShapeType m_shapeID;

	float m_elasticity = 1.0f;

};
