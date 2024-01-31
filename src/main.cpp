#include "Application2D.h"

#include "Common.h"

int main() 
{
	// allocation
	Application2D *app = new Application2D();

	// initialise and loop
	app->run("Physics App", SCREEN_WIDTH, SCREEN_HEIGHT, false);

	// deallocation
	delete app;

	return 0;
}
