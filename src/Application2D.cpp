#include "Application2D.h"

#include <iostream>
#include <vector>
#include <utility>

#include "Common.h"

#include <fmod_errors.h>

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

static bool toggleInfoText = true;

static float aspectRatio;

float tempPoolTableWidth;
float tempPoolTableHeight;

int pottedCount[6]; // Holds how many balls have been potted in each hole.

// Game Objects.
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

// A bunch of resources.
aie::Texture *backgroundImg;
aie::Texture *poolTable;
aie::Texture *poolTableShadow;
aie::Texture *shadowImg;
aie::Texture **ballTextures;

FMOD_SOUND *jazzyBGM = nullptr;

FMOD_SOUND *ballStrikeSND = nullptr;
FMOD_SOUND *ballStrike1SND = nullptr;
FMOD_SOUND *ballStrikeSoftSND = nullptr;
FMOD_SOUND *ballStrikeSoft1SND = nullptr;

FMOD_SOUND *cueStrikeSND = nullptr;

FMOD_SOUND *ballHitWallSND = nullptr;
FMOD_SOUND *ballHitWall1SND = nullptr;

FMOD_SOUND *pottedEmptySND = nullptr;
FMOD_SOUND *pottedSND = nullptr;

// FMOD Stuff.
FMOD_CHANNEL *musicChannel = nullptr;
FMOD_CHANNEL *ambientChannel = nullptr;
FMOD_CHANNEL *sfxChannel = nullptr;

FMOD_CHANNELGROUP *musicChannelGroup = nullptr;
FMOD_CHANNELGROUP *sfxLoudChannelGroup = nullptr;
FMOD_CHANNELGROUP *sfxQuietChannelGroup = nullptr; // 'cause some sound effects aren't as prioritized I suppose.

FMOD_DSP *reverbDSPLoudSFX = nullptr;
FMOD_DSP *reverbDSPQuietSFX = nullptr; // I guess you can't share DSP effects between channel groups, so there's a separate one here.

int GetBallNumber() // Utility function, makes sure the number hasn't already been picked and that it's also not zero.
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

unsigned int Application2D::m_windowWidth = 0;
unsigned int Application2D::m_windowHeight = 0;

float Application2D::m_extents = 0;

Application2D::Application2D() 
{ 
	m_windowWidth = SCREEN_WIDTH;
	m_windowHeight = SCREEN_HEIGHT;

	m_extents = 100;
}
Application2D::~Application2D() 
{ }

bool Application2D::startup() 
{
	// Setup application.
	setVSync(false);
	srand((unsigned int)time(nullptr)); // Set random seed.

	// Setup FMOD.
	FMOD_RESULT fmodResult;
	m_fmodSystem = nullptr;
	fmodResult = FMOD_System_Create(&m_fmodSystem, FMOD_VERSION);
	if (fmodResult != FMOD_OK)
	{
		std::printf("FMOD error! (%d) %s\n", fmodResult, FMOD_ErrorString(fmodResult));
		return false;
	}
	fmodResult = FMOD_System_Init(m_fmodSystem, 256, FMOD_INIT_NORMAL, nullptr);
	if (fmodResult != FMOD_OK)
	{
		std::printf("FMOD error! (%d) %s\n", fmodResult, FMOD_ErrorString(fmodResult));
		return false;
	}

	// Create renderer and scene.
	aspectRatio = getWindowWidth() / (float)getWindowHeight();
	aie::Gizmos::create(255U, 255U, 65535U, 65535U);
	m_2dRenderer = new aie::Renderer2D();

	// Load resources.
	// TODO: A proper resource manager.
	backgroundImg = new aie::Texture("./textures/background.jpg");
	poolTable = new aie::Texture("./textures/table.png");
	poolTableShadow = new aie::Texture("./textures/table_shadow.png");

	tempPoolTableWidth = (m_extents - (m_extents - 140.0f));
	tempPoolTableHeight = (m_extents - (m_extents - 140.0f)) / aspectRatio;

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

	GLOBALS::g_font = new aie::Font("./font/consolas.ttf", 18);

	// Load and Setup Audio.
	// A bit messy.
	// TODO: Error Checking.
	fmodResult = FMOD_System_CreateDSPByType(m_fmodSystem, FMOD_DSP_TYPE_SFXREVERB, &reverbDSPLoudSFX); // Reverb for atmosphere.
	FMOD_DSP_SetWetDryMix(reverbDSPLoudSFX, 0.5f, 0.5f, 0.0f);
	fmodResult = FMOD_System_CreateDSPByType(m_fmodSystem, FMOD_DSP_TYPE_SFXREVERB, &reverbDSPQuietSFX);
	FMOD_DSP_SetWetDryMix(reverbDSPQuietSFX, 0.5f, 0.5f, 0.0f);

	fmodResult = FMOD_System_CreateChannelGroup(m_fmodSystem, "Music", &musicChannelGroup);
	FMOD_ChannelGroup_SetVolume(musicChannelGroup, 0.25f);

	fmodResult = FMOD_System_CreateChannelGroup(m_fmodSystem, "SFX Loud", &sfxLoudChannelGroup);
	FMOD_ChannelGroup_SetVolume(sfxLoudChannelGroup, 0.8f);
	FMOD_ChannelGroup_AddDSP(sfxLoudChannelGroup, 0, reverbDSPLoudSFX);

	fmodResult = FMOD_System_CreateChannelGroup(m_fmodSystem, "SFX Quiet", &sfxQuietChannelGroup);
	FMOD_ChannelGroup_SetVolume(sfxQuietChannelGroup, 0.64f);
	FMOD_ChannelGroup_AddDSP(sfxQuietChannelGroup, 0, reverbDSPQuietSFX);

	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/bgm/jazzy_bgm.mp3", FMOD_DEFAULT | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL, nullptr, &jazzyBGM);
	FMOD_Sound_SetLoopCount(jazzyBGM, -1); // Background music should loop infinitely.

	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_strike.mp3", FMOD_DEFAULT, nullptr, &ballStrikeSND);
	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_strike1.mp3", FMOD_DEFAULT, nullptr, &ballStrike1SND);
	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_strike_soft.mp3", FMOD_DEFAULT, nullptr, &ballStrikeSoftSND);
	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_strike_soft1.mp3", FMOD_DEFAULT, nullptr, &ballStrikeSoft1SND);

	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/cue_strike.mp3", FMOD_DEFAULT, nullptr, &cueStrikeSND);

	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_hit_wall.mp3", FMOD_DEFAULT, nullptr, &ballHitWallSND);
	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_hit_wall1.mp3", FMOD_DEFAULT, nullptr, &ballHitWall1SND);

	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_potted.mp3", FMOD_DEFAULT, nullptr, &pottedSND);
	fmodResult = FMOD_System_CreateSound(m_fmodSystem, "./audio/sfx/ball_potted_empty.mp3", FMOD_DEFAULT, nullptr, &pottedEmptySND);

	FMOD_System_PlaySound(m_fmodSystem, jazzyBGM, musicChannelGroup, false, &musicChannel); // Begin playing background music.
	// TODO: Ball rolling sound, though could be annoying. Also, ambient pub chatter sounds might be a nice addition.

	// Setup scene.
	m_physicsScene = new PhysicsScene();
	m_physicsScene->SetGravity({ 0.0f, -9.81f * 0.0f });
	m_physicsScene->SetTimeStep(PHYSICS_TIMESTEP);

	// Table Boundaries
	planeLeft = new Plane(0.3f, { 1.0f, 0.0f }, -70.0f, { 1, 1, 1, 0 }, (m_extents - (m_extents - 70.0f)) / aspectRatio, 10.0f);
	planeRight = new Plane(0.3f, { -1.0f, 0.0f }, -70.0f, { 1, 1, 1, 0 }, (m_extents - (m_extents - 70.0f)) / aspectRatio, 10.0f);
	planeBottom = new Plane(0.3f, { 0.0f, 1.0f }, -70.0f / aspectRatio, { 1, 1, 1, 0 }, (m_extents - (m_extents - 70.0f)), 10.0f);
	planeTop = new Plane(0.3f, { 0.0f, -1.0f }, -70.0f / aspectRatio, { 1, 1, 1, 0 }, (m_extents - (m_extents - 70.0f)), 10.0f);

	m_physicsScene->AddActors({ planeLeft, planeRight, planeBottom, planeTop });

	// Table walls, unfortunately very messy and a lot of values were found by trial and error.
	const int displayColl = 0;
	OBB *boxL = new OBB({ -65, 0 }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 4, 25 }, { 1, 1, 1, displayColl });
	OBB *boxR = new OBB({ 65, 0 }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 4, 25 }, { 1, 1, 1, displayColl });
	OBB *boxUL = new OBB({ -31, 61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25.5f, 4 }, { 1, 1, 1, displayColl });
	OBB *boxUR = new OBB({ 30.5f, 61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25, 4 }, { 1, 1, 1, displayColl });
	OBB *boxBL = new OBB({ -31, -61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25.5f, 4 }, { 1, 1, 1, displayColl });
	OBB *boxBR = new OBB({ 30.5f, -61 / aspectRatio }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.6f, { 25, 4 }, { 1, 1, 1, displayColl });

	OBB *boxCornerUL1 = new OBB({ -63.63f, 45 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerUL2 = new OBB({ -57, 59 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerUR1 = new OBB({ 63.63f, 45 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerUR2 = new OBB({ 56, 59 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	//
	OBB *boxCornerBL1 = new OBB({ -63.63f, -45 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerBL2 = new OBB({ -57, -59 / aspectRatio }, { 0, 0 }, 0, glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerBR1 = new OBB({ 63.63f, -45 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	OBB *boxCornerBR2 = new OBB({ 56, -59 / aspectRatio }, { 0, 0 }, 0, -glm::quarter_pi<float>(), 0.0f, 0.3f, 0.3f, 0.6f, { 2, 2 }, { 1, 1, 1, displayColl });
	//
	OBB *boxCornerUM1 = new OBB({ 5, 57.7f / aspectRatio }, { 0, 0 }, 0, glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, displayColl });
	OBB *boxCornerUM2 = new OBB({ -5, 57.7f / aspectRatio }, { 0, 0 }, 0, -glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, displayColl });
	OBB *boxCornerBM1 = new OBB({ 5, -57.7f / aspectRatio }, { 0, 0 }, 0, -glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, displayColl });
	OBB *boxCornerBM2 = new OBB({ -5, -57.7f / aspectRatio }, { 0, 0 }, 0, glm::pi<float>() / 7, 0.0f, 0.3f, 0.3f, 0.6f, { 1.5f, 1.5f }, { 1, 1, 1, displayColl });

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
	const float drag = 0.8f;
	cueBall = new Ball({ 0, 0 }, { 0, 0 }, 4.0f, 0.0f, 0.0f, drag, drag, 0.8f, 2.0f, 0, ballTextures[0], m_2dRenderer);
	spring = new Spring(nullptr, cueBall, 8, 32, 128, { 0, 20 });

	cueBall->collisionCallback = std::bind(&Application2D::BallCollided, this, std::placeholders::_1, std::placeholders::_2);

	// TODO: Add an actual cue stick.

	int count = 5; // Amount of steps in the triangle.
	for (int i = (count-1); i >= 0; --i) // Setup ball triangle.
	{
		float offset = 0;
		for (int k = (count-1) - i; k >= 0; --k)
		{
			offset += 0.5f;
		}
		for (int j = i; j >= 0; --j)
		{
			float radius = 2.0f;
			float x = (float)i;
			float y = j + offset;

			float height = count * radius * 2;
			glm::vec2 trianglePosition = { m_extents / 4, -height/2 };

			int num = GetBallNumber();
			aie::Texture *tex = ballTextures[num];

			Ball *newBall = new Ball(
				trianglePosition + glm::vec2(x * radius * 2, y * radius * 2 ),
				{ 0, 0 },
				4.0f, 0.0f, 0.0f, drag, drag, 0.8f,
				radius, num, tex, m_2dRenderer);
			newBall->SetRotationLock(true);

			newBall->collisionCallback = std::bind(&Application2D::BallCollided, this, std::placeholders::_1, std::placeholders::_2);

			m_physicsScene->AddActor(newBall);

			billiards.push_back(newBall);
		}
	}

	// Setup holes.
	// MESSY
	const float holeRadius = 3.5f;
	holeTrigger1 = new Ball({ 0, 33.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 0, shadowImg, m_2dRenderer, false);
	holeTrigger2 = new Ball({ 0, -33.75f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 1, shadowImg, m_2dRenderer, false);
	holeTrigger3 = new Ball({ -63.5f, 32.f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 2, shadowImg, m_2dRenderer, false);
	holeTrigger4 = new Ball({ 62.45f, 32.f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 3, shadowImg, m_2dRenderer, false);
	holeTrigger5 = new Ball({ -63.5f, -32.1f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 4, shadowImg, m_2dRenderer, false);
	holeTrigger6 = new Ball({ 62.45f, -32.1f }, { 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, holeRadius, 5, shadowImg, m_2dRenderer, false);

	holeTrigger1->SetKinematic(true); // I hate this lol
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
	// Cleanup textures.
	for (int i = 0; i < 16; ++i)
	{
		delete ballTextures[i];
		ballTextures[i] = nullptr;
	}
	delete[] ballTextures;
	ballTextures = nullptr;

	delete shadowImg;
	shadowImg = nullptr;
	delete poolTable;
	poolTable = nullptr;
	delete poolTableShadow;
	poolTableShadow = nullptr;
	delete backgroundImg;
	backgroundImg = nullptr;

	delete GLOBALS::g_font;
	GLOBALS::g_font = nullptr;

	// Cleanup Audio.
	FMOD_Sound_Release(ballStrikeSND);
	ballStrikeSND = nullptr;
	FMOD_Sound_Release(ballStrike1SND);
	ballStrike1SND = nullptr;
	FMOD_Sound_Release(ballStrikeSoftSND);
	ballStrikeSoftSND = nullptr;
	FMOD_Sound_Release(ballStrikeSoft1SND);
	ballStrikeSoft1SND = nullptr;

	FMOD_Sound_Release(ballHitWallSND);
	ballHitWallSND = nullptr;
	FMOD_Sound_Release(ballHitWall1SND);
	ballHitWall1SND = nullptr;

	FMOD_Sound_Release(pottedSND);
	pottedSND = nullptr;
	FMOD_Sound_Release(pottedEmptySND);
	pottedEmptySND = nullptr;

	FMOD_Sound_Release(cueStrikeSND);
	cueStrikeSND = nullptr;

	FMOD_Sound_Release(jazzyBGM);
	jazzyBGM = nullptr;

	FMOD_ChannelGroup_Release(musicChannelGroup);
	FMOD_ChannelGroup_Release(sfxLoudChannelGroup);
	FMOD_ChannelGroup_Release(sfxQuietChannelGroup);
	musicChannelGroup = nullptr;
	sfxLoudChannelGroup = nullptr;
	sfxQuietChannelGroup = nullptr;

	FMOD_DSP_Release(reverbDSPLoudSFX);
	FMOD_DSP_Release(reverbDSPQuietSFX);
	reverbDSPLoudSFX = nullptr;
	reverbDSPQuietSFX = nullptr;

	// Shutdown FMOD.
	FMOD_System_Close(m_fmodSystem);
	FMOD_System_Release(m_fmodSystem);
	m_fmodSystem = nullptr;
}

void Application2D::update(float deltaTime) 
{
	aie::Input* input = aie::Input::getInstance();

	aie::Gizmos::clear(); // Clear Gizmos on start of frame.

	// Update the Physics scene.
	m_physicsScene->Update(deltaTime);

	// Update FMOD.
	FMOD_System_Update(m_fmodSystem);

	// Reset.
	if (input->wasKeyReleased(aie::INPUT_KEY_R))
	{
		cueBall->ResetPosition();
		for (Ball *ball : billiards)
		{
			ball->ResetPosition();
		}

		for (int i = 0; i < 6; ++i)
			pottedCount[i] = 0;
	}

	// Toggle info text.
	if (input->wasKeyReleased(aie::INPUT_KEY_G))
		toggleInfoText = !toggleInfoText;

	// Toggle debug.
	if (input->wasKeyReleased(aie::INPUT_KEY_P))
		GLOBALS::g_DEBUG = !GLOBALS::g_DEBUG;

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Application2D::draw() 
{
	m_windowWidth = getWindowWidth();
	m_windowHeight = getWindowHeight();
	aspectRatio = m_windowWidth / (float)m_windowHeight;

	const float scaleFactor = (m_windowWidth / 2) / m_extents;

	// wipe the screen to the background colour
	clearScreen();

	// Begin drawing background
	m_2dRenderer->begin();

	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->setUVRect(0, 0, 32 / scaleFactor * aspectRatio, 32 / scaleFactor);
	m_2dRenderer->drawSprite(backgroundImg, (float)m_windowWidth/2, (float)m_windowHeight /2, (float)m_windowWidth, (float)m_windowHeight, 0.0f, 99.0f);

	m_2dRenderer->end();

	// begin drawing sprites on foreground
	m_2dRenderer->begin();

	// MESSY
	// Draw pool table.
	m_2dRenderer->setUVRect(0, 0, 1, 1);
	const float scaleFactorW = (poolTableShadow->getWidth() / (float)poolTable->getWidth()) * scaleFactor;
	const float scaleFactorH = (poolTableShadow->getHeight() / (float)poolTable->getHeight()) * scaleFactor;
	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->drawSprite(poolTableShadow, (float)m_windowWidth / 2 + 8, (float)m_windowHeight / 2 - 16, tempPoolTableWidth * scaleFactorW, tempPoolTableHeight * scaleFactorH, 0.0f, 98.0f);
	
	m_2dRenderer->setRenderColour(0xFFFFFFFF);
	m_2dRenderer->drawSprite(poolTable, (float)m_windowWidth / 2, (float)m_windowHeight / 2, tempPoolTableWidth * scaleFactor, tempPoolTableHeight * scaleFactor, 0.0f, 98.0f);

	m_physicsScene->Draw(); // Draw the physics scene.

	aie::Gizmos::draw2D(glm::ortho<float>(-m_extents, m_extents, -m_extents / aspectRatio, m_extents / aspectRatio, -1.0f, 1.0f)); // Draw gizmos.

	if (toggleInfoText)
	{
		// output some text, uses the last used colour
		char fps[32];
		sprintf_s(fps, 32, "Application FPS: %i", getFPS());
		m_2dRenderer->setRenderColour(0xFFFFFFFF);
		m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, (float)m_windowHeight - 32);

		sprintf_s(fps, 32, "Physics FPS: %i", m_physicsScene->GetFPS()); // Physics FPS unlikely to drop during normal gameplay unless doing a significant amount of physics calculations per frame.
		m_2dRenderer->drawText(GLOBALS::g_font, fps, 0, (float)m_windowHeight - 64);

		m_2dRenderer->drawText(GLOBALS::g_font, "Hold left click on a ball to drag it, ", 0, (float)m_windowHeight - 96);
		m_2dRenderer->drawText(GLOBALS::g_font, "release left click to throw it!", 0, (float)m_windowHeight - (96+18));
		m_2dRenderer->drawText(GLOBALS::g_font, "Press G to toggle this text!", 0, (float)m_windowHeight - (160-18));
		m_2dRenderer->drawText(GLOBALS::g_font, "Press P to toggle the debug view!", 0, (float)m_windowHeight - (192-18));
		m_2dRenderer->drawText(GLOBALS::g_font, "Press R to reset all balls!", 0, (float)m_windowHeight - (224-18));
		m_2dRenderer->drawText(GLOBALS::g_font, "Press ESC to quit!", 0, (float)m_windowHeight - (256-18));

		if (GLOBALS::g_DEBUG == true)
			m_2dRenderer->drawText(GLOBALS::g_font, "DEBUG VIEW!", 0, (float)m_windowHeight - (320-18));
	}

	// done drawing sprites
	m_2dRenderer->end();
}

glm::vec2 Application2D::ScreenToWorld(glm::vec2 screenPosition)
{
	// Screen space -> World space transformation.
	glm::vec2 worldPosition = screenPosition;
	worldPosition.x -= m_windowWidth / 2;
	worldPosition.y -= m_windowHeight / 2;
	worldPosition.x *= 2.0f * m_extents / m_windowWidth;
	worldPosition.y *= 2.0f * m_extents / (aspectRatio * m_windowHeight);
	return worldPosition;
}

glm::vec2 Application2D::TransformCoordinates(glm::vec2 worldCoordinates)
{
	// World space -> Screen space transformation.
	glm::vec2 transformed;

	const float extents = m_extents;
	float scaleFactor = (m_windowWidth / 2) / extents;

	glm::vec2 transformPosition;
	transformPosition.x = worldCoordinates.x * scaleFactor;
	transformPosition.y = worldCoordinates.y * scaleFactor;

	transformed.x = transformPosition.x + (m_windowWidth / 2);
	transformed.y = transformPosition.y + (m_windowHeight / 2);

	return transformed;
}

void Application2D::BallInHole(PhysicsObject *collisionObj, PhysicsObject *other) // Callback function.
{
	Ball *hole = dynamic_cast<Ball *>(collisionObj);
	Ball *ball = dynamic_cast<Ball *>(other);

	// Do this when ball is potted.
	if (ball != nullptr)
	{
		// BUG: Seems to be a bug where sometimes the ball doesn't lerp to the middle of the hole before it "disappears".
		ball->SetKinematic(true);
		ball->SetIsTrigger(true);
		ball->lerpFinishCallback = [=](PhysicsObject *obj)
		{
			ball->SetCaught(true);
			if (GLOBALS::g_carrying == ball)
				GLOBALS::g_carrying = nullptr;
		};
		ball->LerpToPoint(hole->GetPosition(), 8.0f, 0.05f);

		FMOD_System_PlaySound(m_fmodSystem, pottedEmptySND, sfxQuietChannelGroup, false, &sfxChannel);
		if (pottedCount[hole->GetNumber()] > 0)
			FMOD_System_PlaySound(m_fmodSystem, pottedSND, sfxQuietChannelGroup, false, &sfxChannel);

		pottedCount[hole->GetNumber()]++;
	}
}

void Application2D::BallCollided(PhysicsObject *collisionObj, PhysicsObject *other)
{
	Ball *collBall = dynamic_cast<Ball *>(collisionObj);
	Ball *otherBall = dynamic_cast<Ball *>(other);

	if (collBall != nullptr && otherBall != nullptr)
	{
		if (collBall->IsTrigger() || otherBall->IsTrigger())
			return;
		if (collBall->IsDragging() || otherBall->IsDragging())
			return;
		if (collBall->IsCaught() || otherBall->IsCaught())
			return;

		// Will play the sound effects twice with the balls colliding with each other, might not be preferable.
		if (glm::length(collBall->GetVelocity()) <= 5.0f)
		{
			int r = rand() % 2;
			if (r == 0)
				FMOD_System_PlaySound(m_fmodSystem, ballStrikeSoftSND, sfxQuietChannelGroup, false, &sfxChannel);
			else
				FMOD_System_PlaySound(m_fmodSystem, ballStrikeSoft1SND, sfxQuietChannelGroup, false, &sfxChannel);
		}
		else
		{
			int r = rand() % 2;
			if (r == 0)
				FMOD_System_PlaySound(m_fmodSystem, ballStrikeSND, sfxLoudChannelGroup, false, &sfxChannel);
			else
				FMOD_System_PlaySound(m_fmodSystem, ballStrike1SND, sfxLoudChannelGroup, false, &sfxChannel);
		}
	}

	OBB *otherWall = dynamic_cast<OBB *>(other);
	if (collBall != nullptr && otherWall != nullptr)
	{
		if (collBall->IsTrigger() || otherWall->IsTrigger())
			return;
		if (collBall->IsCaught())
			return;

		int r = rand() % 2;
		if (r == 0)
			FMOD_System_PlaySound(m_fmodSystem, ballHitWallSND, sfxQuietChannelGroup, false, &sfxChannel);
		else
			FMOD_System_PlaySound(m_fmodSystem, ballHitWall1SND, sfxQuietChannelGroup, false, &sfxChannel);
	}
}
