#include "stdafx.h"
#include "main.h"
#include <future>

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


Memory*			m			= new Memory();

Logger			logger;
FileLogger		fileLogger;
driverManager	dMGR;



bool				displayEmptyCars	= false;
bool				displayPlayers		= true;
bool				displayCars			= true;
bool				displayItems		= false;
bool				isRunning			= true;


int					screenX = GetSystemMetrics(SM_CXSCREEN);
int					screenY = GetSystemMetrics(SM_CYSCREEN);
const				MARGINS margins = { -1, -1, -1, -1 };



int main()
{
	int pid;
	cout << "pid?" << endl;
	cin >> pid;
	m->Attach(pid);

}

	/*
	console->sendInput( "Loading virtual driver" );

	WCHAR currentPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, currentPath);
	wstring curPath = wstring( currentPath ) + L"\\Loader\\";

	
	if ( !dMGR.loadDriverless( curPath, L"FLDR.exe" , L"FMGR.sys" ) ) {
		//MessageBox(NULL, L"Couldn't load DRIVER", NULL, NULL), exit(0);
		console->sendInput( "Couldn't load driver! Operations will not work." );
	} else {
		console->sendInput( "Driver created with name: FMGR! Unloading virtual driver." );
	}
	*/

	


