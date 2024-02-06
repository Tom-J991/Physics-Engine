#pragma once

#include "Sphere.h"

#include "Renderer2D.h"
#include "Texture.h"

class Ball : public Sphere
{
public:
	Ball(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, int number, aie::Texture *texture, aie::Renderer2D *renderer);
	virtual ~Ball();

	virtual void Draw() override;

	int GetNumber() { return m_number; }

protected:
	glm::vec2 TransformCoordinates(glm::vec2 coordinates);
	float TransformScale(float scale);

protected:
	aie::Renderer2D *m_2dRenderer = nullptr;
	aie::Texture *m_texture = nullptr;

	int m_number = -1;

};
