#include "Ball.h"

#include "Gizmos.h"

#include "Common.h"

Ball::Ball(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, int number, aie::Texture *texture, aie::Renderer2D *renderer)
	: Sphere(position, velocity, mass, orientation, angularVelocity, linearDrag, angularDrag, elasticity, radius)
	, m_2dRenderer(renderer)
	, m_texture(texture)
	, m_number(number)
{ }
Ball::~Ball()
{ }

void Ball::Draw()
{
	glm::vec2 newPos = TransformCoordinates(m_position);
	float newScale = TransformScale(m_radius * 2);

	m_2dRenderer->setUVRect(0, 0, 1, 1);
	m_2dRenderer->drawSprite(m_texture, newPos.x, newPos.y, newScale, newScale, m_orientation, 0.0f);
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
