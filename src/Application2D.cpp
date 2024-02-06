#include "Application2D.h"

#include <iostream>
#include <vector>

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

#include "Ball.h"

const float extents = 100;
static float aspectRatio;

const float physicsTimeStep = 1 / 60.0f;

Sphere *ball1; // All these instances get deleted by the physics scene when the physics scene is deleted.
Plane *planeLeft;
Plane *planeRight;
Plane *planeBottom;
Plane *planeTop;
Spring *spring;

std::vector<Ball*> billiards;

aie::Texture *backgroundImg;
aie::Texture **ballTextures;

int GetBallNumber()
{
	int randomN = rand() % 16;
	if (randomN == 0)
		return GetBallNumber();
	for (int i = 0; i < billiards.size(); ++i)
	{
		if (billiards[i]->GetNumber() == randomN)
			return GetBallNumber();
	}
	return randomN;
}

Application2D::Application2D() 
{ }
Application2D::~Application2D() 
{ }

bool Application2D::startup() 
{
	srand(time(nullptr));

	aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	aie::Gizmos::create(255U, 255U, 65535U, 65535U);

	m_2dRenderer = new aie::Renderer2D();

	backgroundImg = new aie::Texture("./textures/pool_table.jpg");

	ballTextures = new aie::Texture*[16];
	{
		ballTextures[0] = new aie::Texture("./textures/balls/ball_16.png");
		ballTextures[1] = new aie::Texture("./textures/balls/ball_1.png");
		ballTextures[2] = new aie::Texture("./textures/balls/ball_2.png");
		ballTextures[3] = new aie::Texture("./textures/balls/ball_3.png");
		ballTextures[4] = new aie::Texture("./textures/balls/ball_4.png");
		ballTextures[5] = new aie::Texture("./textures/balls/ball_5.png");
		ballTextures[6] = new aie::Texture("./textures/balls/ball_6.png");
		ballTextures[7] = new aie::Texture("./textures/balls/ball_7.png");
		ballTextures[8] = new aie::Texture("./textures/balls/ball_8.png");
		ballTextures[9] = new aie::Texture("./textures/balls/ball_9.png");
		ballTextures[10] = new aie::Texture("./textures/balls/ball_10.png");
		ballTextures[11] = new aie::Texture("./textures/balls/ball_11.png");
		ballTextures[12] = new aie::Texture("./textures/balls/ball_12.png");
		ballTextures[13] = new aie::Texture("./textures/balls/ball_13.png");
		ballTextures[14] = new aie::Texture("./textures/balls/ball_14.png");
		ballTextures[15] = new aie::Texture("./textures/balls/ball_15.png");
	}

	GLOBALS::g_font = new aie::Font("./font/consolas.ttf", 24);

	m_physicsScene = new PhysicsScene();
	m_physicsScene->SetGravity({ 0.0f, -9.81f * 0.0f });
	m_physicsScene->SetTimeStep(physicsTimeStep);

	ball1 = new Ball({ 0, 0 }, { 0, 0 }, 4.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 2.0f, 0, ballTextures[0], m_2dRenderer);
	planeLeft = new Plane(0.3f, { 1.0f, 0.0f }, -80.0f, { 0.5f, 0.3f, 0.1f, 1 }, (extents - (extents - 80.0f)) / aspectRatio, 10.0f);
	planeRight = new Plane(0.3f, { -1.0f, 0.0f }, -80.0f, { 0.5f, 0.3f, 0.1f, 1 }, (extents - (extents - 80.0f)) / aspectRatio, 10.0f);
	planeBottom = new Plane(0.3f, { 0.0f, 1.0f }, -80.0f / aspectRatio, { 0.5f, 0.3f, 0.1f, 1 }, (extents - (extents - 80.0f)), 10.0f);
	planeTop = new Plane(0.3f, { 0.0f, -1.0f }, -80.0f / aspectRatio, { 0.5f, 0.3f, 0.1f, 1 }, (extents - (extents - 80.0f)), 10.0f);

	spring = new Spring(nullptr, ball1, 8, 0, 1024, { 0, 20 });

	int count = 4;
	for (int i = count; i >= 0; --i)
	{
		float offset = 0;
		for (int k = count - i; k >= 0; --k)
		{
			offset += 0.5f;
		}
		for (int j = i; j >= 0; --j)
		{
			float radius = 2.0f;
			float x = j + offset;
			float y = (float)i;

			float height = (count+1) * radius * 2;
			glm::vec2 trianglePosition = { extents / 4, -height/2 };

			int num = GetBallNumber();
			aie::Texture *tex = ballTextures[num];

			Ball *newBall = new Ball(
				trianglePosition + glm::vec2(y * radius * 2, x * radius * 2 ),
				{ 0, 0 }, 
				4.0f, 0.0f, 0.3f, 0.3f, 0.3f, 0.8f, 
				radius, num, tex, m_2dRenderer);
			newBall->SetRotationLock(true);

			m_physicsScene->AddActor(newBall);

			billiards.push_back(newBall);
		}
	}

	m_physicsScene->AddActors({ ball1, planeLeft, planeRight, planeBottom, planeTop });

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
	static glm::vec2 ball1Offset = { 0, 0 };
	static glm::vec2 previousBall1Pos = ball1Offset;
	static glm::vec2 objectDragVelocity = { 0, 0 };

	//
	if (ball1->IsInside(mouseInWorld))
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

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Application2D::draw() 
{
	// wipe the screen to the background colour
	clearScreen();

	m_2dRenderer->begin();

	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->setUVRect(0, 0, 4 * aspectRatio, 4);
	m_2dRenderer->drawSprite(backgroundImg, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f);

	m_2dRenderer->end();

	// begin drawing sprites
	m_2dRenderer->begin();

	m_physicsScene->Draw();
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
