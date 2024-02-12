#pragma once

#include "Sphere.h"

#include "Renderer2D.h"
#include "Texture.h"

class Ball : public Sphere
{
public:
	Ball(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, int number, aie::Texture *texture, aie::Renderer2D *renderer, bool hasShadow = true);
	virtual ~Ball();

	virtual void FixedUpdate(float timeStep) override;
	virtual void Draw() override;

	virtual void ResetPosition() override;
	
	void SetCaught(const bool caught) { m_caught = caught; }

	bool IsCaught() const { return m_caught; }
	int GetNumber() const { return m_number; }

	void LerpToPoint(glm::vec2 point, float lerpSpeed = 4.0f, float lerpThreshold = 0.01f, bool snapOnFinish = false);

public:
	std::function<void(PhysicsObject *)> lerpFinishCallback; // Function to call when lerp has finished.

protected:
	glm::vec2 TransformCoordinates(glm::vec2 coordinates);
	float TransformScale(float scale);

protected:
	aie::Renderer2D *m_2dRenderer = nullptr;
	aie::Texture *m_texture = nullptr;
	aie::Texture *m_shadowTexture = nullptr;
	bool m_hasShadow; // Draw the shadow, or not.
	
	int m_number = -1;
	bool m_caught = false; // Has the ball been caught? (potted)

	glm::vec2 m_initialPosition;
	float m_initialRadius;

	glm::vec4 m_fadeToBlack = { 1, 1, 1, 1 }; // Fade color.

	// Lerp stuff.
	bool m_isLerping = false;
	glm::vec2 m_lerpPoint;
	float m_lerpSpeed = 4.0f;
	float m_lerpThreshold = 0.01f;
	bool m_lerpSnap = false;

	// Dragging ball stuff.
	bool ballDragging = false;
	glm::vec2 ballOffset = { 0, 0 };
	glm::vec2 previousBallPos = ballOffset;
	glm::vec2 dragDisplacement = { 0, 0 };

};
