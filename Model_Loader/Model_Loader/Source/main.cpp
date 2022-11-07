// Main.cpp
#include "Scene.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

/// <summary>
/// Main function called when the program is run
/// </summary>
int main()
{
	Scene* pScene = Scene::GetInstance();
	if (pScene)
	{
		// do initial setup for the scene
		bool bIsInitialised = pScene->Initialise();
		if (bIsInitialised)
		{
			bool bShouldClose = false;
			while (!bShouldClose)
			{
				// call update on the scene
				bShouldClose = pScene->Update();
				// call render on the scene
				pScene->Render();
			}
			// call the function to clean everything down
			pScene->Deinitialise();
		}
	}
	delete pScene;

	return 0;
}