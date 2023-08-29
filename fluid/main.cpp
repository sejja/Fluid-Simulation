
#include "Fluid.h"

#pragma warning(push, 0)
#include <windows.h>
#include <thread>
#include <iostream>
#pragma warning(pop)

Fluid * FLUID = NULL;

void InitializeDirectX();
int RunDirectX();

// Frame Update Function - Called Every Frame
void CALLBACK OnFrameMove( double /*fTime*/, float /*fElapsedTime*/, void* /*pUserContext*/ )
{
	FLUID->Update( FluidTimestep );
}

// main
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	FLUID = new Fluid();
	FLUID->Create( 1.6f, 1.2f );

#ifdef _DEBUG
	FLUID->Fill(0.2f);
#else
	FLUID->Fill(0.5f);
#endif

	InitializeDirectX();
	// Setup Platform/Renderer backends

	int retval = RunDirectX();
	delete FLUID; FLUID = NULL;
	return retval;
}
