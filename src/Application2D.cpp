#include "Application2D.h"

#include "Common.h"

#include "Gizmos.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"

#include "RigidBody.h"
#include "Sphere.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Sphere *ball1;
Sphere *ball2;

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
	m_physicsScene->SetGravity({ 0.0f, 0.0f });
	m_physicsScene->SetTimeStep(0.01f);

	ball1 = new Sphere({ -4, 0 }, { 0, 0 }, 4.0f, 4, { 1, 0, 0, 1 });
	ball2 = new Sphere({ 4, 0 }, { 0, 0 }, 4.0f, 4, { 0, 1, 0, 1 });

	m_physicsScene->AddActor(ball1);
	m_physicsScene->AddActor(ball2);

	return true;
}

void Application2D::shutdown() 
{
	delete GLOBALS::g_font;
}

void Application2D::update(float deltaTime) 
{
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_L))
	{
		ball1->ApplyForceToActor(ball2, { -2.0f, 0.0f });
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
	//setBackgroundColour(1.0f, 1.0f, 1.0f);
	clearScreen();

	// begin drawing sprites
	m_2dRenderer->begin();

	static float aspectRatio = 16.f / 9.f;
	aie::Gizmos::draw2D(glm::ortho<float>(-100, 100, -100 / aspectRatio, 100 / aspectRatio, -1.0f, 1.0f)); // Draw gizmos.

	// output some text, uses the last used colour
	char fps[32];
	sprintf_s(fps, 32, "FPS: %i", getFPS());
	m_2dRenderer->setRenderColour(0x00FF00FF);
	m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, SCREEN_HEIGHT - 32);
	m_2dRenderer->drawText(GLOBALS::g_font, "Press ESC to quit!", 0, SCREEN_HEIGHT - 64);

	// done drawing sprites
	m_2dRenderer->end();
}
