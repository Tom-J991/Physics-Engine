#include "Application2D.h"

#include "Common.h"

#include "Texture.h"
#include "Font.h"
#include "Input.h"

Application2D::Application2D() 
{ }
Application2D::~Application2D() 
{ }

bool Application2D::startup() 
{
	m_2dRenderer = new aie::Renderer2D();

	GLOBALS::g_font = new aie::Font("./font/consolas.ttf", 24);

	return true;
}

void Application2D::shutdown() 
{
	delete GLOBALS::g_font;
}

void Application2D::update(float deltaTime) 
{
	aie::Input* input = aie::Input::getInstance();

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
	
	// output some text, uses the last used colour
	char fps[32];
	sprintf_s(fps, 32, "FPS: %i", getFPS());
	m_2dRenderer->setRenderColour(0x00FF00FF);
	m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, SCREEN_HEIGHT - 32);
	m_2dRenderer->drawText(GLOBALS::g_font, "Press ESC to quit!", 0, SCREEN_HEIGHT - 64);

	// done drawing sprites
	m_2dRenderer->end();
}
