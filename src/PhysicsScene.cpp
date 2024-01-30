#include "PhysicsScene.h"

#include "Gizmos.h"

#include "PhysicsObject.h"
#include "RigidBody.h"
#include "Sphere.h"
#include "Plane.h"

PhysicsScene::PhysicsScene()
	: m_timeStep { 0.01f }
{ 
	m_gravity = { 0, 0 };
}
PhysicsScene::~PhysicsScene()
{ 
	// Deallocate.
	for (auto p_actor : m_actors)
	{
		delete p_actor;
	}
}

void PhysicsScene::AddActor(PhysicsObject *actor)
{
	m_actors.push_back(actor);
	actor->SetPhysicsScene(this);
}
void PhysicsScene::RemoveActor(PhysicsObject *actor)
{
	for (std::vector<PhysicsObject *>::iterator i = m_actors.begin(); i != m_actors.end(); ++i)
	{
		if (*i == actor)
		{
			actor->SetPhysicsScene(nullptr);
			m_actors.erase(i);
			return;
		}
	}
}

typedef bool(*fn)(PhysicsObject *, PhysicsObject *); // C-style function pointer typedef.
static fn collisionFunctionArray[] = // Holds function pointers to collision resolution for each pair of rigid body objects, the index positions in this array are important.
{
	PhysicsScene::Plane2Plane, PhysicsScene::Plane2Sphere,
	PhysicsScene::Sphere2Plane, PhysicsScene::Sphere2Sphere
};

#include <iostream>
void PhysicsScene::Update(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	accumulatedTime += deltaTime;

	while (accumulatedTime >= m_timeStep) // Fixed time step.
	{
		for (auto p_actor : m_actors)
		{
			p_actor->FixedUpdate(m_timeStep);
		}
		accumulatedTime -= m_timeStep;

		// Do collision check.
		int actorCount = (int)m_actors.size();
		for (int outer = 0; outer < actorCount - 1; outer++)
		{
			for (int inner = outer + 1; inner < actorCount; inner++)
			{
				PhysicsObject *object1 = m_actors[outer];
				PhysicsObject *object2 = m_actors[inner];

				int shapeID1 = object1->GetShapeID();
				int shapeID2 = object2->GetShapeID();

				int functionIndex = (shapeID1 * ShapeType::SHAPE_COUNT) + shapeID2;
				fn collisionFunction = collisionFunctionArray[functionIndex]; // Get function pointer for proper collision handling.
				if (collisionFunction != nullptr)
				{
					collisionFunction(object1, object2);
				}
			}
		}

		float energy = GetTotalEnergy(); // Diagnostics.
		std::cout << energy << std::endl;
	}
}

void PhysicsScene::Draw()
{
	for (auto p_actor : m_actors)
	{
		p_actor->Draw();
	}
}

float PhysicsScene::GetTotalEnergy()
{
	float total = 0.0f;
	for (std::vector<PhysicsObject *>::iterator i = m_actors.begin(); i != m_actors.end(); i++)
	{
		PhysicsObject *obj = *i;
		total += obj->GetEnergy();
	}
	return total;
}

// Collision stuff.
bool PhysicsScene::Plane2Plane(PhysicsObject *obj1, PhysicsObject *obj2)
{
	return false; // Don't need collision handling for static objects.
}

bool PhysicsScene::Plane2Sphere(PhysicsObject *obj1, PhysicsObject *obj2)
{
	return Sphere2Plane(obj2, obj1);
}

bool PhysicsScene::Sphere2Plane(PhysicsObject *obj1, PhysicsObject *obj2)
{
	Sphere *sphere = dynamic_cast<Sphere *>(obj1);
	Plane *plane = dynamic_cast<Plane *>(obj2);

	if (sphere != nullptr && plane != nullptr)
	{
		float distanceToCheck = glm::dot(sphere->GetPosition(), plane->GetNormal()) - plane->GetDistance(); // The distance between the sphere's center and the plane.
		float intersection = distanceToCheck - sphere->GetRadius(); // The distance betwen the sphere's surface and the plane.
		float velocityOutOfPlane = glm::dot(sphere->GetVelocity(), plane->GetNormal()); // Is the sphere travelling into or out of the plane?
		if (intersection < 0 && velocityOutOfPlane < 0)
		{
			plane->ResolveCollision(sphere);
			return true;
		}
	}

	return false;
}

bool PhysicsScene::Sphere2Sphere(PhysicsObject *obj1, PhysicsObject *obj2)
{
	Sphere *sphere1 = dynamic_cast<Sphere *>(obj1);
	Sphere *sphere2 = dynamic_cast<Sphere *>(obj2);

	if (sphere1 != nullptr && sphere2 != nullptr)
	{
		float distanceToCheck = glm::distance(sphere1->GetPosition(), sphere2->GetPosition());
		if (distanceToCheck < (sphere1->GetRadius() + sphere2->GetRadius())) // Compares distance between each sphere to the sum of their radius'
		{
			sphere1->ResolveCollision(sphere2);
			return true;
		}
	}

	return false;
}
