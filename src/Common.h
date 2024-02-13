#pragma once

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define PHYSICS_TIMESTEP (1 / 144.f)

class Ball; // Forward declaration.

namespace GLOBALS
{
	// Shouldn't really use globals like this but it makes it a lot easier.
	inline bool g_DEBUG = false; // Debug view flag.

	inline aie::Font *g_font; // Global reference to font texture.
	
	inline Ball *g_carrying = nullptr; // Global reference to what the cursor is currently carrying, might be temporary.

}
