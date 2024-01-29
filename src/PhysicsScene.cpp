#include "PhysicsScene.h"

#include "Gizmos.h"

#include "PhysicsObject.h"

PhysicsScene::PhysicsScene()
	: m_gravity { 0, 0 }
	, m_timeStep { 0.01f }
{ }
PhysicsScene::~PhysicsScene()
{ }

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

	while (accumulatedTime >= m_timeStep)
	{
		for (auto p_actor : m_actors)
		{
			p_actor->FixedUpdate(m_gravity, m_timeStep);
		}
		accumulatedTime -= m_timeStep;
	}
}

void PhysicsScene::Draw()
{
	for (auto p_actor : m_actors)
	{
		p_actor->Draw();
	}
}
