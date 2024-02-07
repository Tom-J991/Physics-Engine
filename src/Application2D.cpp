#include "Application2D.h"

#include <iostream>
#include <vector>
#include <utility>

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

const float physicsTimeStep = 1 / 144.0f;

Ball *cueBall; // All these instances get deleted by the physics scene when the physics scene is deleted.
Spring *spring;
Plane *planeLeft;
Plane *planeRight;
Plane *planeBottom;
Plane *planeTop;

Ball *holeTrigger1;
Ball *holeTrigger2;
Ball *holeTrigger3;
Ball *holeTrigger4;
Ball *holeTrigger5;
Ball *holeTrigger6;

std::vector<Ball*> billiards;

aie::Texture *backgroundImg;
aie::Texture *poolTable;
aie::Texture *poolTableShadow;
aie::Texture *shadowImg;
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
	setVSync(false);

	srand(time(nullptr));

	aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	aie::Gizmos::create(255U, 255U, 65535U, 65535U);

	m_2dRenderer = new aie::Renderer2D();

	backgroundImg = new aie::Texture("./textures/background.jpg");
	poolTable = new aie::Texture("./textures/table.png");
	poolTableShadow = new aie::Texture("./textures/table_shadow.png");

	shadowImg = new aie::Texture("./textures/shadow.png");

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

	// Table Boundaries
	planeLeft = new Plane(0.3f, { 1.0f, 0.0f }, -70.0f, { 1, 1, 1, 0 }, (extents - (extents - 70.0f)) / aspectRatio, 10.0f);
	planeRight = new Plane(0.3f, { -1.0f, 0.0f }, -70.0f, { 1, 1, 1, 0 }, (extents - (extents - 70.0f)) / aspectRatio, 10.0f);
	planeBottom = new Plane(0.3f, { 0.0f, 1.0f }, -70.0f / aspectRatio, { 1, 1, 1, 0 }, (extents - (extents - 70.0f)), 10.0f);
	planeTop = new Plane(0.3f, { 0.0f, -1.0f }, -70.0f / aspectRatio, { 1, 1, 1, 0 }, (extents - (extents - 70.0f)), 10.0f);

	m_physicsScene->AddActors({ planeLeft, planeRight, planeBottom, planeTop });

	// Table Borders, unfortunately messy and a lot of values were found by trial and error.
	OBB *boxL = new OBB({ -65, 0 }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 4, 25 }, { 1, 1, 1, 0 });
	OBB *boxR = new OBB({ 65, 0 }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 4, 25 }, { 1, 1, 1, 0 });
	OBB *boxUL = new OBB({ -31, 61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25.5f, 4 }, { 1, 1, 1, 0 });
	OBB *boxUR = new OBB({ 30.5f, 61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25, 4 }, { 1, 1, 1, 0 });
	OBB *boxBL = new OBB({ -31, -61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25.5f, 4 }, { 1, 1, 1, 0 });
	OBB *boxBR = new OBB({ 30.5f, -61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25, 4 }, { 1, 1, 1, 0 });

	OBB *boxCornerUL1 = new OBB({ -63.63f, 45 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerUL2 = new OBB({ -57, 59 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerUR1 = new OBB({ 63.63f, 45 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerUR2 = new OBB({ 56, 59 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	//
	OBB *boxCornerBL1 = new OBB({ -63.63f, -45 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerBL2 = new OBB({ -57, -59 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerBR1 = new OBB({ 63.63f, -45 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	OBB *boxCornerBR2 = new OBB({ 56, -59 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, 0 });
	//
	OBB *boxCornerUM1 = new OBB({ 5, 57.7f / aspectRatio }, { 0, 0 }, 0, glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, 0 });
	OBB *boxCornerUM2 = new OBB({ -5, 57.7f / aspectRatio }, { 0, 0 }, 0, -glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, 0 });
	OBB *boxCornerBM1 = new OBB({ 5, -57.7f / aspectRatio }, { 0, 0 }, 0, -glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, 0 });
	OBB *boxCornerBM2 = new OBB({ -5, -57.7f / aspectRatio }, { 0, 0 }, 0, glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, 0 });

	boxL->SetKinematic(true);
	boxR->SetKinematic(true);
	boxUL->SetKinematic(true);
	boxUR->SetKinematic(true);
	boxBL->SetKinematic(true);
	boxBR->SetKinematic(true);

	boxCornerUL1->SetKinematic(true);
	boxCornerUL2->SetKinematic(true);
	boxCornerUR1->SetKinematic(true);
	boxCornerUR2->SetKinematic(true);
	boxCornerBL1->SetKinematic(true);
	boxCornerBL2->SetKinematic(true);
	boxCornerBR1->SetKinematic(true);
	boxCornerBR2->SetKinematic(true);
	boxCornerUM1->SetKinematic(true);
	boxCornerUM2->SetKinematic(true);
	boxCornerBM1->SetKinematic(true);
	boxCornerBM2->SetKinematic(true);

	m_physicsScene->AddActors({ boxL, boxR, boxUL, boxUR, boxBL, boxBR });
	m_physicsScene->AddActors(
		{ boxCornerUL1, boxCornerUL2, boxCornerUR1, boxCornerUR2, 
		boxCornerBL1, boxCornerBL2, boxCornerBR1, boxCornerBR2, 
		boxCornerUM1, boxCornerUM2, boxCornerBM1, boxCornerBM2 });

	// Game objects.
	cueBall = new Ball({ 0, -20 }, { 0, 0 }, 4.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 2.0f, 0, ballTextures[0], m_2dRenderer);
	spring = new Spring(nullptr, cueBall, 8, 32, 128, { 0, 20 });

	int count = 4;
	for (int i = count; i >= 0; --i) // Setup ball triangle.
	{
		float offset = 0;
		for (int k = count - i; k >= 0; --k)
		{
			offset += 0.5f;
		}
		for (int j = i; j >= 0; --j)
		{
			float radius = 2.0f;
			float x = (float)i;
			float y = j + offset;

			float height = (count+1) * radius * 2;
			glm::vec2 trianglePosition = { extents / 4, -height/2 };

			int num = GetBallNumber();
			aie::Texture *tex = ballTextures[num];

			Ball *newBall = new Ball(
				trianglePosition + glm::vec2(x * radius * 2, y * radius * 2 ),
				{ 0, 0 }, 
				4.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.8f, 
				radius, num, tex, m_2dRenderer);
			newBall->SetRotationLock(true);

			m_physicsScene->AddActor(newBall);

			billiards.push_back(newBall);
		}
	}

	// Setup holes.
	const float holeRadius = 3.5f;
	holeTrigger1 = new Ball({ 0, 33.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);
	holeTrigger2 = new Ball({ 0, -33.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);
	holeTrigger3 = new Ball({ -63.75f, 31.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);
	holeTrigger4 = new Ball({ 62.65f, 31.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);
	holeTrigger5 = new Ball({ -63.75f, -31.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);
	holeTrigger6 = new Ball({ 62.65f, -31.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, -1, shadowImg, m_2dRenderer, false);

	holeTrigger1->SetKinematic(true);
	holeTrigger1->SetIsTrigger(true);
	holeTrigger2->SetKinematic(true);
	holeTrigger2->SetIsTrigger(true);
	holeTrigger3->SetKinematic(true);
	holeTrigger3->SetIsTrigger(true);
	holeTrigger4->SetKinematic(true);
	holeTrigger4->SetIsTrigger(true);
	holeTrigger5->SetKinematic(true);
	holeTrigger5->SetIsTrigger(true);
	holeTrigger6->SetKinematic(true);
	holeTrigger6->SetIsTrigger(true);

	holeTrigger1->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);
	holeTrigger2->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);
	holeTrigger3->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);
	holeTrigger4->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);
	holeTrigger5->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);
	holeTrigger6->triggerEnter = std::bind(&Application2D::BallInHole, this, std::placeholders::_1, std::placeholders::_2);

	m_physicsScene->AddActors({ cueBall, holeTrigger1, holeTrigger2, holeTrigger3, holeTrigger4, holeTrigger5, holeTrigger6 });

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
	if (cueBall->IsCaught() == false)
	{
		if (cueBall->IsInside(mouseInWorld))
		{
			cueBall->SetColour({ 1, 1, 0, 1 });
			if (ball1Dragging == false && input->isMouseButtonDown(0))
			{
				ball1Offset = cueBall->GetPosition() - mouseInWorld;
				ball1Dragging = true;
			}
		}
		else
		{
			cueBall->SetColour({ 1, 0, 0, 1 });
		}
		if (input->isMouseButtonDown(0) && ball1Dragging == true)
		{
			cueBall->SetColour({ 1, 1, 1, 1 });

			glm::vec2 newPos = mouseInWorld + ball1Offset;
			objectDragVelocity = newPos - previousBall1Pos;
			previousBall1Pos = newPos;

			cueBall->SetKinematic(true);
			cueBall->SetPosition(newPos);
		}
		if (input->isMouseButtonUp(0) && ball1Dragging == true)
		{
			glm::vec2 deltaVel = objectDragVelocity - cueBall->GetVelocity();
			glm::vec2 accel = deltaVel / physicsTimeStep;
			cueBall->SetKinematic(false);
			cueBall->ApplyForce(cueBall->GetMass() * accel, { 0, 0 });
			objectDragVelocity = { 0, 0 };
			ball1Dragging = false;
		}
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

	// Begin drawing background
	m_2dRenderer->begin();

	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->setUVRect(0, 0, 4 * aspectRatio, 4);
	m_2dRenderer->drawSprite(backgroundImg, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 99.0f);

	m_2dRenderer->end();

	// begin drawing sprites on foreground
	m_2dRenderer->begin();

	const float scaleFactorW = (poolTableShadow->getWidth() / (float)poolTable->getWidth());
	const float scaleFactorH = (poolTableShadow->getHeight() / (float)poolTable->getHeight());
	m_2dRenderer->setUVRect(0, 0, 1, 1);
	m_2dRenderer->setRenderColour(0xFFFFFF80);
	m_2dRenderer->drawSprite(poolTableShadow, SCREEN_WIDTH / 2 + 8, SCREEN_HEIGHT / 2 - 16, SCREEN_WIDTH * scaleFactorW - 384, SCREEN_HEIGHT * scaleFactorH - 384 / aspectRatio, 0.0f, 98.0f);
	
	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->drawSprite(poolTable, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 384, SCREEN_HEIGHT - 384 / aspectRatio, 0.0f, 98.0f);

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

void Application2D::BallInHole(PhysicsObject *collisionObj, PhysicsObject *other)
{
	Ball *hole = dynamic_cast<Ball *>(collisionObj);
	Ball *ball = dynamic_cast<Ball *>(other);

	if (ball != nullptr)
	{
		ball->SetKinematic(true);
		ball->lerpFinishCallback = [=](PhysicsObject *obj)
		{
			ball->SetCaught(true);
		};
		ball->LerpToPoint(hole->GetPosition(), 8.0f, 0.1f);
	}
}
