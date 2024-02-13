#include "PhysicsScene.h"

#include "Gizmos.h"

#include "PhysicsObject.h"
#include "RigidBody.h"
#include "Sphere.h"
#include "Plane.h"
#include "OBB.h"

PhysicsScene::PhysicsScene()
	: m_timeStep { 1/30.f }
{ 
	m_gravity = { 0, 0 };
}
PhysicsScene::~PhysicsScene()
{ 
	// Deallocate all actors.
	for (auto p_actor : m_actors)
	{
		delete p_actor;
	}
}

typedef bool(*fn)(PhysicsObject *, PhysicsObject *); // C-style function pointer typedef.
static fn collisionFunctionArray[] = // Holds function pointers to collision resolution for each pair of rigid body objects, the index positions in this array are important.
{
	PhysicsScene::Plane2Plane, PhysicsScene::Plane2Sphere, PhysicsScene::Plane2Box,
	PhysicsScene::Sphere2Plane, PhysicsScene::Sphere2Sphere, PhysicsScene::Sphere2Box,
	PhysicsScene::Box2Plane, PhysicsScene::Box2Sphere, PhysicsScene::Box2Box,
};

unsigned int frames = 0;
double fpsInterval = 0.0;

#include <iostream>
void PhysicsScene::Update(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	accumulatedTime += deltaTime;

	while (accumulatedTime >= m_timeStep) // Fixed time step.
	{
		frames++;
		fpsInterval += m_timeStep;

		for (auto p_actor : m_actors)
		{
			p_actor->FixedUpdate(m_timeStep);
		}

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

				if (shapeID1 < 0 || shapeID2 < 0)
					continue;

				int functionIndex = (shapeID1 * ShapeType::SHAPE_COUNT) + shapeID2;
				fn collisionFunction = collisionFunctionArray[functionIndex]; // Get function pointer for proper collision handling.
				if (collisionFunction != nullptr)
				{
					collisionFunction(object1, object2);
				}
			}
		}

		float energy = GetTotalEnergy(); // Diagnostics.
		//std::cout << "Total Energy in Physics Scene: " << energy << std::endl;

		accumulatedTime -= m_timeStep;
	}

	if (fpsInterval >= 1.0f) // Get FPS.
	{
		m_fps = frames;
		frames = 0;
		fpsInterval -= 1.0f;
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
void PhysicsScene::ApplyContactForces(RigidBody *body1, RigidBody *body2, glm::vec2 normal, float penetration)
{
	if (body1 == nullptr) // Can't apply force to null object.
		return;
	if ((body1 && body1->IsTrigger()) || (body2 && body2->IsTrigger())) // Don't continue if at least one of the objects is a trigger.
		return;
	if ((body1 && body1->IsKinematic()) && (body2 && body2->IsKinematic())) // Don't continue if both objects are kinematic, kinematic objects shouldn't be effected by outside forces.
		return;

	float body2Mass = body2 ? body2->GetMass() : INT_MAX; // Use passed in mass for body2, otherwise it's near infinite.
	float body1Factor = body2Mass / (body1->GetMass() + body2Mass);
	// Push objects out of each other.
	if (body1->IsKinematic() == false) // Don't push a kinematic object.
		body1->SetPosition(body1->GetPosition() - body1Factor * normal * penetration);
	if (body2 && body2->IsKinematic() == false) // Same here.
		body2->SetPosition(body2->GetPosition() + (1 - body1Factor) * normal * penetration);
}

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
		if (intersection < 0 && velocityOutOfPlane < 0) // Sphere is intersecting plane.
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
		float penetration = sphere1->GetRadius() + sphere2->GetRadius() - distanceToCheck; // Compares distance between each sphere to the sum of their radius'
		if (penetration > 0) // Spheres are penetrating, resolve collision.
		{
			sphere1->ResolveCollision(sphere2, 0.5f * (sphere1->GetPosition() + sphere2->GetPosition()), nullptr, penetration);
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

		glm::vec2 planeOrigin = plane->GetNormal() * plane->GetDistance(); // Get point on plane.

		// Iterate through corners of the box to check if it has hit the plane.
		for (float x = -box->GetExtents().x; x < box->GetWidth(); x += box->GetWidth())
		{
			for (float y = -box->GetExtents().y; y < box->GetHeight(); y += box->GetHeight())
			{
				glm::vec2 p = box->GetPosition() + x * box->GetLocalX() + y * box->GetLocalY(); // Find the position of this corner in world space.
				float distanceToCheck = glm::dot(p - planeOrigin, plane->GetNormal());

				// Find velocity of the corner in world space.
				glm::vec2 displacement = x * box->GetLocalX() + y * box->GetLocalY();
				glm::vec2 pointVelocity = box->GetVelocity() + box->GetAngularVelocity() * glm::vec2(-displacement.y, displacement.x);
				float velocityIntoPlane = glm::dot(pointVelocity, plane->GetNormal());

				if (distanceToCheck < 0 && velocityIntoPlane <= 0) // Is the corner moving into the plane?
				{
					// Count the corner that has hit the plane.
					numContacts++;
					contact += p;
					contactV += velocityIntoPlane;
				}
			}
		}

		if (numContacts > 0) // A contact with the plane has been found.
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
		// Transform sphere coordinates to be within the box's coordinate space.
		glm::vec2 circlePosWorld = sphere->GetPosition() - box->GetPosition();
		glm::vec2 circlePosBox = glm::vec2(glm::dot(circlePosWorld, box->GetLocalX()), glm::dot(circlePosWorld, box->GetLocalY()));
		// The closest point to the circle's centre on the box.
		glm::vec2 closestPointOnBoxBox = circlePosBox;
		glm::vec2 extents = box->GetExtents();
		if (closestPointOnBoxBox.x < -extents.x) closestPointOnBoxBox.x = -extents.x;
		if (closestPointOnBoxBox.x > extents.x) closestPointOnBoxBox.x = extents.x;
		if (closestPointOnBoxBox.y < -extents.y) closestPointOnBoxBox.y = -extents.y;
		if (closestPointOnBoxBox.y > extents.y) closestPointOnBoxBox.y = extents.y;
		// Convert back to world space.
		glm::vec2 closestPointOnBoxWorld = box->GetPosition() + closestPointOnBoxBox.x * box->GetLocalX() + closestPointOnBoxBox.y * box->GetLocalY();
		glm::vec2 circleToBox = sphere->GetPosition() - closestPointOnBoxWorld;

		float penetration = sphere->GetRadius() - glm::length(circleToBox); // Compare radius of sphere to the distance from the closest point on the box.
		if (penetration > 0) // Sphere and box are penetrating, resolve collision.
		{
			glm::vec2 direction = glm::length(circleToBox) == 0 ? circleToBox : glm::normalize(circleToBox);
			glm::vec2 contact = closestPointOnBoxWorld;
			box->ResolveCollision(sphere, contact, &direction, penetration);
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
		float penetration = 0;
		int numContacts = 0;
		box1->CheckBoxCorners(*box2, contact, numContacts, penetration, norm); // Check contacts for box1 against box2.
		if (box2->CheckBoxCorners(*box1, contact, numContacts, penetration, norm)) // Check contacts for box2 against box1.
		{
			norm = -norm;
		}
		if (penetration > 0) // Boxes are penetrating, resolve collision.
		{
			box1->ResolveCollision(box2, contact / float(numContacts), &norm, penetration);
		}
		return true;
	}

	return false;
}

void PhysicsScene::AddActor(PhysicsObject *actor)
{
	m_actors.push_back(actor);
	actor->SetPhysicsScene(this);
}

void PhysicsScene::AddActors(std::initializer_list<PhysicsObject *> actorList)
{
	for (std::initializer_list<PhysicsObject *>::iterator i = actorList.begin(); i != actorList.end(); ++i)
	{
		PhysicsObject *actor = *i;
		AddActor(actor);
	}
}
void PhysicsScene::AddActors(std::vector<PhysicsObject *> actorList)
{
	for (std::vector<PhysicsObject *>::iterator i = actorList.begin(); i != actorList.end(); ++i)
	{
		PhysicsObject *actor = *i;
		AddActor(actor);
	}
}

void PhysicsScene::RemoveActor(PhysicsObject *actor)
{
	for (std::vector<PhysicsObject *>::iterator i = m_actors.begin(); i != m_actors.end(); ++i)
	{
		if (*i == actor)
		{
			actor->SetPhysicsScene(nullptr);
			m_actors.erase(i);
			break;
		}
	}
}

void PhysicsScene::RemoveActors(std::initializer_list<PhysicsObject *> actorList)
{
	for (std::initializer_list<PhysicsObject *>::iterator i = actorList.begin(); i != actorList.end(); ++i)
	{
		PhysicsObject *actor = *i;
		RemoveActor(actor);
	}
}
void PhysicsScene::RemoveActors(std::vector<PhysicsObject *> actorList)
{
	for (std::vector<PhysicsObject *>::iterator i = actorList.begin(); i != actorList.end(); ++i)
	{
		PhysicsObject *actor = *i;
		RemoveActor(actor);
	}
}
