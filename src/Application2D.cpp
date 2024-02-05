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

#include <glm/glm.hpp>
#include <glm/ext.hpp>

const float extents = 100;
const float aspectRatio = 16.f / 9.f;

Sphere *ball1; // All these instances get deleted by the physics scene when the physics scene is deleted.
Sphere *ball2;
OBB *box;
Plane *planeLeft;
Plane *planeRight;
Plane *planeBottom;
Plane *planeTop;
Spring *spring;

Application2D::Application2D() 
{ }
Application2D::~Application2D() 
{ }

bool Application2D::startup() 
{
	aie::Gizmos::create(255U, 255U, 65535U, 65535U);

	m_2dRenderer = new aie::Renderer2D();

	GLOBALS::g_font = new aie::Font("./font/consolas.ttf", 24);

	m_physicsScene = new PhysicsScene();
	m_physicsScene->SetGravity({ 0.0f, -9.81f * 4.0f });
	m_physicsScene->SetTimeStep(1/60.f);

	ball1 = new Sphere({ 0, 20 }, { 0, 0 }, 4.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 4);
	ball2 = new Sphere({ 0, 0 }, { 40, 0 }, 4.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 4, { 1, 0, 1, 1 });
	//box = new OBB({ 0, 12 }, { 40, 0 }, 6.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.3f, { 4, 12 });
	planeLeft = new Plane(0.3f, { 1.0f, 0.0f }, -50.0f * 1.7777f);
	planeRight = new Plane(0.3f, { -1.0f, 0.0f }, -50.0f * 1.7777f);
	planeBottom = new Plane(0.3f, { 0.0f, 1.0f }, -50.0f);
	planeTop = new Plane(0.3f, { 0.0f, -1.0f }, -50.0f);

	spring = new Spring(ball1, ball2, 0, 6, 5);

	ball1->SetKinematic(true);

	m_physicsScene->AddActors({ ball1, ball2, planeLeft, planeRight, planeBottom, planeTop, spring });

	return true;
}

void Application2D::shutdown() 
{
	delete GLOBALS::g_font;
}

void Application2D::update(float deltaTime) 
{
	aie::Input* input = aie::Input::getInstance();

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
