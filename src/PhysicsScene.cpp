#include "PhysicsScene.h"

#include "Gizmos.h"

#include "PhysicsObject.h"
#include "RigidBody.h"
#include "Sphere.h"

PhysicsScene::PhysicsScene()
	: m_gravity { 0, 0 }
	, m_timeStep { 0.01f }
{ }
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
}
void PhysicsScene::RemoveActor(PhysicsObject *actor)
{
	for (std::vector<PhysicsObject *>::iterator i = m_actors.begin(); i != m_actors.end(); ++i)
	{
		if (*i == actor)
		{
			m_actors.erase(i);
			return;
		}
	}
}

void PhysicsScene::Update(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	accumulatedTime += deltaTime;

	while (accumulatedTime >= m_timeStep) // Fixed time step.
	{
		for (auto p_actor : m_actors)
		{
			p_actor->FixedUpdate(m_gravity, m_timeStep);
		}
		accumulatedTime -= m_timeStep;

		// Do collision check.
		int actorCount = m_actors.size();
		for (int outer = 0; outer < actorCount - 1; outer++)
		{
			for (int inner = outer + 1; inner < actorCount; inner++)
			{
				PhysicsObject *object1 = m_actors[outer];
				PhysicsObject *object2 = m_actors[inner];
				Sphere2Sphere(object1, object2);
			}
		}
	}
}

void PhysicsScene::Draw()
{
	for (auto p_actor : m_actors)
	{
		p_actor->Draw();
	}
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
			// Sets velocity to 0 for now.
			sphere1->SetVelocity({ 0.0f, 0.0f });
			sphere2->SetVelocity({ 0.0f, 0.0f });

			return true;
		}
	}

	return false;
}
