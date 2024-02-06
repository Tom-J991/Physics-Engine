#include "Application2D.h"

#include <iostream>

#include "Common.h"

#include "Gizmos.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"

#include "RigidBody.h"
#include "Sphere.h"
#include "Plane.h"
#include "OBB.h"
#include "Spring.h"

const float extents = 100;
const float aspectRatio = 16.f / 9.f;

const float physicsTimeStep = 1 / 60.0f;

Sphere *ball1; // All these instances get deleted by the physics scene when the physics scene is deleted.
Sphere *ball2;
OBB *box;
Plane *planeLeft;
Plane *planeRight;
Plane *planeBottom;
Plane *planeTop;
Spring *spring;

aie::Texture *m_backgroundImg;

glm::vec4 HSV2RGB(float hue, float saturation, float value)
{
	float r, g, b;

	int i = glm::floor(hue * 6);
	float f = hue * 6.0f - i;
	float p = value * (1 - saturation);
	float q = value * (1 - f * saturation);
	float t = value * (1 - (1 - f) * saturation);

	switch (i % 6)
	{
		case 0: { r = value; g = t; b = p; } break;
		case 1: { r = q; g = value; b = p; } break;
		case 2: { r = p; g = value; b = t; } break;
		case 3: { r = p; g = q; b = value; } break;
		case 4: { r = t; g = p; b = value; } break;
		case 5: { r = value; g = p; b = q; } break;
	}

	return { r, g, b, 1.0f };
}

Application2D::Application2D() 
{ }
Application2D::~Application2D() 
{ }

bool Application2D::startup() 
{
	aie::Gizmos::create(255U, 255U, 65535U, 65535U);

	m_2dRenderer = new aie::Renderer2D();

	m_backgroundImg = new aie::Texture("./textures/underwater.png");

	GLOBALS::g_font = new aie::Font("./font/consolas.ttf", 24);

	m_physicsScene = new PhysicsScene();
	m_physicsScene->SetGravity({ 0.0f, -9.81f * 8.0f });
	m_physicsScene->SetTimeStep(physicsTimeStep);

	ball1 = new Sphere({ 20, -40 }, { -100, 100 }, 40.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 8);
	ball2 = new Sphere({ 0, 0 }, { 100, -100 }, 12.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 6, { 1, 0, 1, 1 });
	box = new OBB({ 40, 20 }, { 40, 0 }, 6.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.3f, { 4, 12 });
	planeLeft = new Plane(0.3f, { 1.0f, 0.0f }, -50.0f * aspectRatio, { 0, 1, 1, 1 }, (extents - 50.0f));
	planeRight = new Plane(0.3f, { -1.0f, 0.0f }, -50.0f * aspectRatio, { 0, 1, 1, 1 }, (extents - 50.0f));
	planeBottom = new Plane(0.3f, { 0.0f, 1.0f }, -50.0f, { 0, 1, 1, 1 }, (extents - 50.0f) * aspectRatio);
	planeTop = new Plane(0.3f, { 0.0f, -1.0f }, -50.0f, { 0, 1, 1, 1 }, (extents - 50.0f) * aspectRatio);

	spring = new Spring(nullptr, ball1, 8, 0, 1024, { 0, 20 });

	m_physicsScene->AddActors({ box, ball1, ball2, spring, planeLeft, planeRight, planeBottom, planeTop });

	for (int i = 0; i < 128*1; ++i)
	{
		Sphere *newSphere = new Sphere(
			{ 80 - rand() % 160, 45 - rand() % 90 }, 
			{ 50 - rand() % 100, 30 - rand() % 60 }, 
			1, 0.0f, 0.3f, 0.3f, 0.3f, 0.3f, 
			1 + rand() % 3, 
			HSV2RGB((rand() % 255) / 255.0f, 1.0f, 1.0f));
		m_physicsScene->AddActor(newSphere);
	}

	return true;
}

void Application2D::shutdown() 
{
	delete GLOBALS::g_font;
}

void Application2D::update(float deltaTime) 
{
	aie::Input* input = aie::Input::getInstance();

	int mouseX, mouseY;
	int mouseDeltaX, mouseDeltaY;
	input->getMouseXY(&mouseX, &mouseY);
	input->getMouseDelta(&mouseDeltaX, &mouseDeltaY);

	// Messy mouse dragging logic.
	glm::vec2 mouseInWorld = ScreenToWorld({ mouseX, mouseY });

	static bool ball1Dragging = false;
	static bool ball2Dragging = false;
	static glm::vec2 ball1Offset = { 0, 0 };
	static glm::vec2 ball2Offset = { 0, 0 };
	static glm::vec2 previousBall1Pos = ball1Offset;
	static glm::vec2 previousBall2Pos = ball2Offset;
	static glm::vec2 objectDragVelocity = { 0, 0 };

	if (ball2->IsInside(mouseInWorld) && ball1Dragging == false)
	{
		ball2->SetColour({ 1, 1, 0, 1 });
		if (ball2Dragging == false && input->isMouseButtonDown(0))
		{
			ball2Offset = ball2->GetPosition() - mouseInWorld;
			ball2Dragging = true;
		}
	}
	else
	{
		ball2->SetColour({ 1, 0, 1, 1 });
	}
	if (input->isMouseButtonDown(0) && ball2Dragging == true)
	{
		ball2->SetColour({ 1, 1, 1, 1 });

		glm::vec2 newPos = mouseInWorld + ball2Offset;
		objectDragVelocity = newPos - previousBall2Pos;
		previousBall2Pos = newPos;

		ball2->SetKinematic(true);
		ball2->SetPosition(newPos);
	}
	if (input->isMouseButtonUp(0) && ball2Dragging == true)
	{
		glm::vec2 deltaVel = objectDragVelocity - ball2->GetVelocity();
		glm::vec2 accel = deltaVel / physicsTimeStep;
		ball2->SetKinematic(false);
		ball2->ApplyForce(ball2->GetMass() * accel, { 0, 0 });
		objectDragVelocity = { 0, 0 };
		ball2Dragging = false;
	}

	//
	if (ball1->IsInside(mouseInWorld) && ball2Dragging == false)
	{
		ball1->SetColour({ 1, 1, 0, 1 });
		if (ball1Dragging == false && input->isMouseButtonDown(0))
		{
			ball1Offset = ball1->GetPosition() - mouseInWorld;
			ball1Dragging = true;
		}
	}
	else
	{
		ball1->SetColour({ 1, 0, 0, 1 });
	}
	if (input->isMouseButtonDown(0) && ball1Dragging == true)
	{
		ball1->SetColour({ 1, 1, 1, 1 });

		glm::vec2 newPos = mouseInWorld + ball1Offset;
		objectDragVelocity = newPos - previousBall1Pos;
		previousBall1Pos = newPos;

		ball1->SetKinematic(true);
		ball1->SetPosition(newPos);
	}
	if (input->isMouseButtonUp(0) && ball1Dragging == true)
	{
		glm::vec2 deltaVel = objectDragVelocity - ball1->GetVelocity();
		glm::vec2 accel = deltaVel / physicsTimeStep;
		ball1->SetKinematic(false);
		ball1->ApplyForce(ball1->GetMass() * accel, { 0, 0 });
		objectDragVelocity = { 0, 0 };
		ball1Dragging = false;
	}

	aie::Gizmos::clear(); // Clear Gizmos.

	// Physics scene.
	m_physicsScene->Update(deltaTime);
	m_physicsScene->Draw();

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Application2D::draw() 
{
	// wipe the screen to the background colour
	setBackgroundColour(0.392f, 0.584f, 0.929f);
	clearScreen();

	m_2dRenderer->begin();

	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->setUVRect(0, 0, 1, 1);
	m_2dRenderer->drawSprite(m_backgroundImg, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f);

	m_2dRenderer->end();

	// begin drawing sprites
	m_2dRenderer->begin();

	aie::Gizmos::draw2D(glm::ortho<float>(-extents, extents, -extents / aspectRatio, extents / aspectRatio, -1.0f, 1.0f)); // Draw gizmos.

	// output some text, uses the last used colour
	char fps[32];
	sprintf_s(fps, 32, "Application FPS: %i", getFPS());
	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, SCREEN_HEIGHT - 32);

	sprintf_s(fps, 32, "Physics FPS: %i", m_physicsScene->GetFPS());
	m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, SCREEN_HEIGHT - 64);

	m_2dRenderer->drawText(GLOBALS::g_font, "Press ESC to quit!", 0, SCREEN_HEIGHT - 96);

	// done drawing sprites
	m_2dRenderer->end();
}

glm::vec2 Application2D::ScreenToWorld(glm::vec2 screenPosition)
{
	glm::vec2 worldPosition = screenPosition;
	worldPosition.x -= SCREEN_WIDTH / 2;
	worldPosition.y -= SCREEN_HEIGHT / 2;
	worldPosition.x *= 2.0f * extents / SCREEN_WIDTH;
	worldPosition.y *= 2.0f * extents / (aspectRatio * SCREEN_HEIGHT);
	return worldPosition;
}
