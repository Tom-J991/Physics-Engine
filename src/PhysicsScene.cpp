#include "PhysicsScene.h"

#include "Gizmos.h"

#include "PhysicsObject.h"
#include "RigidBody.h"
#include "Sphere.h"
#include "Plane.h"
#include "OBB.h"

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
	PhysicsScene::Plane2Plane, PhysicsScene::Plane2Sphere, PhysicsScene::Plane2Box,
	PhysicsScene::Sphere2Plane, PhysicsScene::Sphere2Sphere, PhysicsScene::Sphere2Box,
	PhysicsScene::Box2Plane, PhysicsScene::Box2Sphere, PhysicsScene::Box2Box,
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

bool PhysicsScene::Plane2Box(PhysicsObject *obj1, PhysicsObject *obj2)
{
	return Box2Plane(obj2, obj1);
}

bool PhysicsScene::Sphere2Plane(PhysicsObject *obj1, PhysicsObject *obj2)
{
	Sphere *sphere = dynamic_cast<Sphere *>(obj1);
	Plane *plane = dynamic_cast<Plane *>(obj2);

	if (sphere != nullptr && plane != nullptr)
	{
		glm::vec2 collisionNormal = plane->GetNormal();
		float distanceToCheck = glm::dot(sphere->GetPosition(), plane->GetNormal()) - plane->GetDistance(); // The distance between the sphere's center and the plane.
		float intersection = distanceToCheck - sphere->GetRadius(); // The distance betwen the sphere's surface and the plane.
		float velocityOutOfPlane = glm::dot(sphere->GetVelocity(), plane->GetNormal()); // Is the sphere travelling into or out of the plane?
		if (intersection < 0 && velocityOutOfPlane < 0)
		{
			glm::vec2 contact = sphere->GetPosition() + (collisionNormal * -sphere->GetRadius());
			plane->ResolveCollision(sphere, contact);
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
			sphere1->ResolveCollision(sphere2, 0.5f * (sphere1->GetPosition() + sphere2->GetPosition()));
			return true;
		}
	}

	return false;
}

bool PhysicsScene::Sphere2Box(PhysicsObject *obj1, PhysicsObject *obj2)
{
	return Box2Sphere(obj2, obj1);
}

bool PhysicsScene::Box2Plane(PhysicsObject *obj1, PhysicsObject *obj2)
{
	OBB *box = dynamic_cast<OBB *>(obj1);
	Plane *plane = dynamic_cast<Plane *>(obj2);

	if (box != nullptr && plane != nullptr)
	{
		int numContacts = 0;
		glm::vec2 contact(0, 0);
		float contactV = 0;

		glm::vec2 planeOrigin = plane->GetNormal() * plane->GetDistance();

		for (float x = -box->GetExtents().x; x < box->GetWidth(); x += box->GetWidth())
		{
			for (float y = -box->GetExtents().y; y < box->GetHeight(); y += box->GetHeight())
			{
				glm::vec2 p = box->GetPosition() + x * box->GetLocalX() + y * box->GetLocalY();
				float distanceToCheck = glm::dot(p - planeOrigin, plane->GetNormal());

				glm::vec2 displacement = x * box->GetLocalX() + y * box->GetLocalY();
				glm::vec2 pointVelocity = box->GetVelocity() + box->GetAngularVelocity() * glm::vec2(-displacement.y, displacement.x);
				float velocityIntoPlane = glm::dot(pointVelocity, plane->GetNormal());

				if (distanceToCheck < 0 && velocityIntoPlane <= 0)
				{
					numContacts++;
					contact += p;
					contactV += velocityIntoPlane;
				}
			}
		}

		if (numContacts > 0)
		{
			plane->ResolveCollision(box, contact / (float)numContacts);
			return true;
		}
	}

	return false;
}

bool PhysicsScene::Box2Sphere(PhysicsObject *obj1, PhysicsObject *obj2)
{
	OBB *box = dynamic_cast<OBB *>(obj1);
	Sphere *sphere = dynamic_cast<Sphere *>(obj2);

	if (box != nullptr && sphere != nullptr)
	{
		glm::vec2 circlePosWorld = sphere->GetPosition() - box->GetPosition();
		glm::vec2 circlePosBox = glm::vec2(glm::dot(circlePosWorld, box->GetLocalX()), glm::dot(circlePosWorld, box->GetLocalY()));
		glm::vec2 closestPointOnBoxBox = circlePosBox;
		glm::vec2 extents = box->GetExtents();
		if (closestPointOnBoxBox.x < -extents.x) closestPointOnBoxBox.x = -extents.x;
		if (closestPointOnBoxBox.x > extents.x) closestPointOnBoxBox.x = extents.x;
		if (closestPointOnBoxBox.y < -extents.y) closestPointOnBoxBox.y = -extents.y;
		if (closestPointOnBoxBox.y > extents.y) closestPointOnBoxBox.y = extents.y;
		glm::vec2 closestPointOnBoxWorld = box->GetPosition() + closestPointOnBoxBox.x * box->GetLocalX() + closestPointOnBoxBox.y * box->GetLocalY();
		glm::vec2 circleToBox = sphere->GetPosition() - closestPointOnBoxWorld;
		if (glm::length(circleToBox) < sphere->GetRadius())
		{
			glm::vec2 direction = glm::normalize(circleToBox);
			glm::vec2 contact = closestPointOnBoxWorld;
			box->ResolveCollision(sphere, contact, &direction);
			return true;
		}
	}

	return false;
}

bool PhysicsScene::Box2Box(PhysicsObject *obj1, PhysicsObject *obj2)
{
	OBB *box1 = dynamic_cast<OBB *>(obj1);
	OBB *box2 = dynamic_cast<OBB *>(obj2);

	if (box1 != nullptr && box2 != nullptr)
	{
		glm::vec2 boxPos = box2->GetPosition() - box1->GetPosition();
		glm::vec2 norm = { 0.0f, 0.0f };
		glm::vec2 contact = { 0.0f, 0.0f };
		float pen = 0;
		int numContacts = 0;
		box1->CheckBoxCorners(*box2, contact, numContacts, pen, norm);
		if (box2->CheckBoxCorners(*box1, contact, numContacts, pen, norm))
		{
			norm = -norm;
		}
		if (pen > 0)
		{
			box1->ResolveCollision(box2, contact / float(numContacts), &norm);
		}
		return true;
	}

	return false;
}
