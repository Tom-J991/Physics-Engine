#include "Ball.h"

#include "Gizmos.h"
#include "Input.h"

#include "Common.h"

#include "Application2D.h"

#include <glm/ext.hpp>

Ball::Ball(glm::vec2 position, glm::vec2 velocity, float mass, float orientation, float angularVelocity, float linearDrag, float angularDrag, float elasticity, float radius, int number, aie::Texture *texture, aie::Renderer2D *renderer, bool hasShadow)
	: Sphere(position, velocity, mass, orientation, angularVelocity, linearDrag, angularDrag, elasticity, radius)
	, m_2dRenderer(renderer)
	, m_texture(texture)
	, m_number(number)
	, m_hasShadow(hasShadow)
{ 
	// Load extra resources.
	if (m_hasShadow)
		m_shadowTexture = new aie::Texture("./textures/shadow.png");
}
Ball::~Ball()
{ 
	Sphere::~Sphere();

	// Cleanup extra resources.
	if (m_shadowTexture != nullptr)
	{
		delete m_shadowTexture;
		m_shadowTexture = nullptr;
	}
}

void Ball::FixedUpdate(float timeStep)
{
	aie::Input *input = aie::Input::getInstance();

	if (IsTrigger() == false) // Don't do dragging if ball is a trigger.
	{
		int mouseX, mouseY;
		input->getMouseXY(&mouseX, &mouseY); // Get the mouse position on screen.

		glm::vec2 mouseInWorld = Application2D::ScreenToWorld({ mouseX, mouseY }); // Get the mouse position within world space.

		if (IsCaught() == false) // Don't allow dragging if the ball is caught.
		{
			if (IsInside(mouseInWorld)) // Is the mouse overlapping the ball?
			{
				if (ballDragging == false && GLOBALS::g_carrying == nullptr && input->isMouseButtonDown(0)) // Has just clicked on ball, not carrying any other object.
				{
					// Set is carrying.
					GLOBALS::g_carrying = this;
					ballDragging = true;

					SetVelocity({ 0, 0 }); // Reset velocity.
					ballOffset = GetPosition() - mouseInWorld; // Get offset from cursor so it doesn't snap directly to cursor position, looks awkward otherwise.
				}
			}
			if (input->isMouseButtonDown(0) && ballDragging == true) // Is dragging ball.
			{
				// Move ball to new position on drag.
				glm::vec2 newPos = mouseInWorld + ballOffset;
				dragDisplacement = newPos - previousBallPos;

				SetPosition(newPos);

				previousBallPos = newPos;
			}
			if (input->isMouseButtonUp(0) && ballDragging == true) // Ball has been released.
			{
				glm::vec2 accel = dragDisplacement / timeStep;
				SetVelocity(accel / GetMass()); // Throw ball when let go, keep velocity.
				// BUG: Often velocity can annoyingly be set to zero despite the mouse movement, 
				// possibly because the new position and the previous position can be the same on a given frame so the displacement is zero? Probably worth looking into.
				
				// Reset variables.
				dragDisplacement = { 0, 0 };
				ballDragging = false;
				GLOBALS::g_carrying = nullptr;
			}
		}
	}

	Sphere::FixedUpdate(timeStep); // Do Sphere physics.

	// If the ball has been caught.
	if (m_caught == true)
	{
		// Animation so it looks like the ball is falling into the pot.
		m_fadeToBlack.a -= timeStep * 2.0f; // Fade out.
		if (m_fadeToBlack.a <= 0.0f)
			m_fadeToBlack.a = 0.0f; // Clamp to 0.

		m_radius -= timeStep; // Shrink ball.
		if (m_radius <= 0.0f) // Clamp to 0.
			m_radius = 0.0f;
	}

	if (m_isLerping) // Handle lerping ball position.
	{
		// Lerp ball position to point.
		m_position.x = std::lerp(m_position.x, m_lerpPoint.x, timeStep * m_lerpSpeed);
		m_position.y = std::lerp(m_position.y, m_lerpPoint.y, timeStep * m_lerpSpeed);

		if (m_position.x >= m_lerpPoint.x - m_lerpThreshold && m_position.x <= m_lerpPoint.x + m_lerpThreshold) // Since lerping like this is technically incorrect and won't ever get to the goal, check if it's within a certain threshold to finish.
		{
			// If ball within threshold then snap it to the point.
			if (m_lerpSnap == true)
				m_position = m_lerpPoint;

			if (lerpFinishCallback)
				lerpFinishCallback(this); // Lerp has finished, do this.

			m_isLerping = false; // Reset flag.
		}
	}
}

void Ball::Draw()
{
	if (m_fadeToBlack.a <= 0.0f) // Don't even attempt to draw if invisible.
		return;

	glm::vec2 newPos = TransformCoordinates(m_position); // Transform coordinates since the 2D Renderer is within bounds of x = 0 <-> Client Area Width, y = 0 <-> Client Area Height, no other way to match the Gizmos projection matrix without hacking it into the Bootstrap source.
	float newScale = TransformScale(m_radius * 2); // Same for the scale.

	m_2dRenderer->setUVRect(0, 0, 1, 1);
	if (m_hasShadow && m_shadowTexture != nullptr) // Draw shadow beneath object if it has one.
	{
		m_2dRenderer->setRenderColour(0xFFFFFF80);
		m_2dRenderer->drawSprite(m_shadowTexture, newPos.x + 2, newPos.y - 3, newScale * 1.5f, newScale * 1.5f, m_orientation, 0.0f);
	}

	// Draw ball sprite.
	m_2dRenderer->setRenderColour(m_fadeToBlack.r, m_fadeToBlack.g, m_fadeToBlack.b, m_fadeToBlack.a);
	m_2dRenderer->drawSprite(m_texture, newPos.x, newPos.y, newScale, newScale, m_orientation, 0.0f);

	if (IsTrigger() && GLOBALS::g_DEBUG == true) // Draw debug circle over trigger.
	{
		aie::Gizmos::add2DCircle(m_position, m_radius, 24, { 0.0f, 1.0f, 1.0f, 0.0f });
	}
}

void Ball::LerpToPoint(glm::vec2 point, float lerpSpeed, float lerpThreshold, bool snapOnFinish)
{
	// Just setup lerp. It is handled in fixed update.
	m_lerpSnap = snapOnFinish;
	m_lerpSpeed = lerpSpeed;
	m_lerpThreshold = lerpThreshold;
	m_lerpPoint = point;
	m_isLerping = true;
}

glm::vec2 Ball::TransformCoordinates(glm::vec2 coordinates)
{
	// Screen space -> World space transformation.
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
	// Screen space -> World space transformation.
	float scaled = 0.0f;

	const float extents = 100;
	float scaleFactor = (SCREEN_WIDTH/2) / extents;
	scaled = scale * scaleFactor;

	return scaled;
}
