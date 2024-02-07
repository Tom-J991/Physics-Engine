#include "Ball.h"

#include "Gizmos.h"

#include "Common.h"

#include <glm/ext.hpp>

Ball::Ball(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, int number, aie::Texture *texture, aie::Renderer2D *renderer, bool hasShadow)
	: Sphere(position, velocity, mass, orientation, angularVelocity, linearDrag, angularDrag, elasticity, radius)
	, m_2dRenderer(renderer)
	, m_texture(texture)
	, m_number(number)
	, m_hasShadow(hasShadow)
{ 
	if (m_hasShadow)
		m_shadowTexture = new aie::Texture("./textures/shadow.png");
}
Ball::~Ball()
{ }

void Ball::FixedUpdate(float timeStep)
{
	Sphere::FixedUpdate(timeStep);

	if (m_caught == true)
	{
		m_fadeToBlack.a -= timeStep * 2.0f;
		if (m_fadeToBlack.a <= 0.0f)
			m_fadeToBlack.a = 0.0f;

		m_radius -= timeStep;
		if (m_radius <= 0.0f)
			m_radius = 0.0f;
	}

	if (m_isLerping)
	{
		m_position.x = std::lerp(m_position.x, m_lerpPoint.x, timeStep * m_lerpSpeed);
		m_position.y = std::lerp(m_position.y, m_lerpPoint.y, timeStep * m_lerpSpeed);
		if (m_position.x >= m_lerpPoint.x - m_lerpThreshold && m_position.x <= m_lerpPoint.x + m_lerpThreshold)
		{
			if (m_lerpSnap == true)
				m_position = m_lerpPoint;
			if (lerpFinishCallback)
				lerpFinishCallback(this);
		}
	}
}

void Ball::Draw()
{
	if (m_fadeToBlack.a <= 0.0f)
		return;

	glm::vec2 newPos = TransformCoordinates(m_position);
	float newScale = TransformScale(m_radius * 2);

	m_2dRenderer->setUVRect(0, 0, 1, 1);

	if (m_hasShadow)
	{
		m_2dRenderer->setRenderColour(0xFFFFFF80);
		m_2dRenderer->drawSprite(m_shadowTexture, newPos.x + 2, newPos.y - 3, newScale * 1.5f, newScale * 1.5f, m_orientation, 0.0f);
	}

	m_2dRenderer->setRenderColour(m_fadeToBlack.r, m_fadeToBlack.g, m_fadeToBlack.b, m_fadeToBlack.a);
	m_2dRenderer->drawSprite(m_texture, newPos.x, newPos.y, newScale, newScale, m_orientation, 0.0f);
}

void Ball::LerpToPoint(glm::vec2 point, float lerpSpeed, float lerpThreshold, bool snapOnFinish)
{
	m_lerpSnap = snapOnFinish;
	m_lerpSpeed = lerpSpeed;
	m_lerpThreshold = lerpThreshold;
	m_lerpPoint = point;
	m_isLerping = true;
}

glm::vec2 Ball::TransformCoordinates(glm::vec2 coordinates)
{
	glm::vec2 transformed;

	const float extents = 100;
	float scaleFactor = (SCREEN_WIDTH/2) / extents;

	glm::vec2 transformPosition;
	transformPosition.x = m_position.x * scaleFactor;
	transformPosition.y = (m_position.y) * scaleFactor;

	transformed.x = transformPosition.x + (SCREEN_WIDTH/2);
	transformed.y = transformPosition.y + (SCREEN_HEIGHT/2);

	return transformed;
}

float Ball::TransformScale(float scale)
{
	float scaled = 0.0f;

	const float extents = 100;
	float scaleFactor = (SCREEN_WIDTH/2) / extents;
	scaled = scale * scaleFactor;

	return scaled;
}
