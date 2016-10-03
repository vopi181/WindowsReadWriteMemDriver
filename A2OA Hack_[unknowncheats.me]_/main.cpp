#include "main.h"
#include "Drawer.h"
#include <future>

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void inputHanlder( char* command );

rManager*		d3			= new rManager();
d3Console*		console		= new d3Console( 10,10,800,700 );
Memory*			m			= new Memory();

Logger			logger;
FileLogger		fileLogger;
driverManager	dMGR;
LPD3DXFONT		font;
D3D9Menu		d3d9;


bool				displayEmptyCars	= false;
bool				displayPlayers		= true;
bool				displayCars			= true;
bool				displayItems		= false;
bool				isRunning			= true;


int					screenX = GetSystemMetrics(SM_CXSCREEN);
int					screenY = GetSystemMetrics(SM_CYSCREEN);
const				MARGINS margins = { -1, -1, -1, -1 };


VOID renderOverlay() {
	d3->clear();

	d3d9.render();
	console->render();

	d3->present();
}

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	while (!m->isAttached())
		m->Attach(m->getProcessIdFromName("ArmA2OA.exe")), Sleep(1000);

	WNDCLASSEX		cx;
	HWND			hWnd		= NULL;

	cx.cbClsExtra = NULL;
	cx.cbSize = sizeof(WNDCLASSEX);
	cx.cbWndExtra = NULL;
	cx.hbrBackground = (HBRUSH)0;
	cx.hCursor = LoadCursor(NULL, IDC_ARROW);
	cx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	cx.hInstance = hInstance;
	cx.lpfnWndProc = WndProc;
	cx.lpszClassName = L"A2";
	cx.lpszMenuName = NULL;
	cx.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&cx))
		MessageBox(NULL, L"Couldn't register class", NULL, NULL), exit(0);

	hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, L"A2", L"A2", WS_POPUP, 0, 0, screenX, screenY, NULL, NULL, hInstance, NULL);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, ULW_COLORKEY | LWA_ALPHA);

	ShowWindow(hWnd, nShowCmd);

	if ( !d3->Initilize(hWnd, screenX, screenY) )
		MessageBox(NULL, L"Couldn't initialize device", NULL, NULL), exit(0);

	if (!console->initilize(d3, hInstance))
		MessageBox(NULL, L"Couldn't initialize console", NULL, NULL), exit(0);

	if ( !d3d9.initDevice(d3) )
		MessageBox(NULL, L"Couldn't initialize device", NULL, NULL), exit(0);


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

	d3d9.addMenu("Exploder's Menu", 10, 10, VK_OEM_PLUS);

	d3d9.addTab("Vision");
	d3d9.addTab("Position");
	d3d9.addTab("Stats");
	d3d9.addTab("Frames");
	d3d9.addTab("Other");


	// VISION
	d3d9.addSlider(d3d9.addItem("XRAY", 10000, "1000m", render, m, true, false, 0), "XRAY", 50000);

	d3d9.addCheckBox("Players", &displayPlayers, 0);

	d3d9.addCheckBox("Populated Cars", &displayCars, 0);
	d3d9.addCheckBox("Empty Cars", &displayEmptyCars, 0);

	//POSITION
	d3d9.addSlider(d3d9.addItem("Teleport", 1, "1m", teleport, m, false, true, 1), "Teleport", 10);
	d3d9.addSlider(d3d9.addItem("TeleportUnsafe", 1, "1m", teleportUnsafe, m, false, true, 1), "Teleport Unsafe", 10);

	//STATS
	d3d9.addSlider(d3d9.addItem("Indirect Damage", 1, "1m", setIndirect, m, false, false, 2), "Indirect Damage", 500);
	d3d9.addSlider(d3d9.addItem("Indirect Range", 1, "1m", setRange, m, false, false, 2), "Indirect Range", 200);
	d3d9.addSlider(d3d9.addItem("Damage", 1, "1", setDamage, m, false, false, 2), "Damage", 50);

	d3d9.addItem("Reset Stats", 0, "NULL", resetStats, m, false, false, 2);

	//FRAMES
	d3d9.addTextBox(d3d9.addItem("Frame Player", 0, "NULL", framePlayer, m, false, true, 3), "Frame Player", setPlayer);
	d3d9.addTextBox(d3d9.addItem("Kill Player", 0, "NULL", killPlayer, m, false, true, 3), "Kill Player", setPlayer);

	//OTHER
	d3d9.addTextBox(d3d9.addItem("Spawn Weapon", 0, "NULL", spawnWeapon, m, false, false, 4), "Weapon", setWeapon);
	d3d9.addTextBox(d3d9.addItem("Spawn Consumable", 0, "NULL", spawnAmmunation, m, false, false, 4), "Consumable", setConsumable);
	d3d9.addTextBox(d3d9.addItem("Spawn Ammo", 0, "NULL", spawnAmmunation, m, false, false, 4), "Ammo", setAmmunation);

	d3d9.addItem("Unlock Vehicle", 0, "NULL", unlockCar, m, false, false, 4);

	logger.registerInput(inputHanlder);
	logger.handleInput();

	d3d9.createFont( "Arial", 14, 0, &font );

	console->sendInput( "Process ready!" );

	MSG msg;
	SIZE_T	delay	= clock();
	while ( isRunning )
	{
		if( clock() - delay > 5000 )
			isRunning = m->isRunning();

		renderOverlay();

		console->receveInput();
		d3d9.handleInput();

		while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	d3->~rManager();

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		renderOverlay();
		break;
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &margins);
		break;
	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void inputHanlder(char* command)
{
	if (!command || strlen(command) < 1)
		return;

	char* context = NULL;
	char* token = NULL;

	vector<string> splitString;

	token = strtok_s(command, " ", &context);
	if (!token)
		return;

	while (token != NULL)
	{
		splitString.push_back(token);
		token = strtok_s(NULL, " ", &context);
	}

	if (!strcmp(splitString.at(0).c_str(), "list"))
	{
		if (splitString.size() > 2)
		{
			if (splitString.at(1) == "weapon")
				listWeapon(splitString.at(2));
			else
				listConsumable(splitString.at(2));
		} else {
			cout << "list - [args] - 1. weapon, consumable 2. TYPE (rifle, mgun, pistol) (magazine, handgranade) (*)ALL" << endl;
		}
	} else if (!strcmp(splitString.at(0).c_str(), "spawn"))
	{
		if (splitString.size() > 3)
		{
			int ID = 0, amount = 0;

			try {
				ID = stoi(splitString.at(2));
				amount = stoi(splitString.at(3));
			} catch (...)
			{
				return;
			}

			if (splitString.at(1) == "weapon")
			{
				spawnWeaponConsole(ID, amount);
			} else {
				spawnConsumableConsole(ID, amount);
			}

		} else {
			cout << "spawn - [args] - 1.TYPE 2. ITEM ID, 3. AMOUNT" << endl;
		}
	} else if (!strcmp(splitString.at(0).c_str(), "find"))
	{
		if (splitString.size() > 2)
		{
			if (splitString.at(1) == "weapon")
			{
				findWeapon(splitString.at(2));
			} else {
				findConsumable(splitString.at(2));
			}

		} else {
			cout << "find - [args] - 1. TYPE (weapon, consumable) 2. ITEM NAME" << endl;
		}
	} else if (!strcmp(splitString.at(0).c_str(), "terminate"))
	{
		cout << "Terminating scripts!" << endl;

		INT counter = 0;
		do {
			cout << "Iteration: " << counter << endl;
			terminateScripts();
			counter++;

			std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
		} while ( counter > 4 );	

		cout << "Done." << endl;
	} else if (!strcmp(splitString.at(0).c_str(), "kill"))
	{
		static std::future<VOID>	future;
		static bool					run = true;

		if (future.valid()) {
			auto status = future.wait_for(std::chrono::milliseconds(0));
			if (status != std::future_status::ready) {
				run = false;
				cout << "Stopping KILL mode!" << endl;

				return VOID();
			} else {
				run = true;
			}
		}

		if (splitString.size() > 2)
		{
			if (splitString.at(1).length() > 0 && splitString.at(2).length() > 0)
			{
				try {
					vector<int> targetList;


					stringstream ss(splitString.at(1));
					string ID;
					while (std::getline(ss, ID, ',')) {
						targetList.push_back(stoi(ID));
					}

					//INT targetIndex = stoi( splitString.at(1) );
					INT frameIndex = stoi(splitString.at(2));

					future = std::async(killPlayerConsole, targetList, frameIndex, &run);

				} catch (std::exception e)
				{
					cout << "Error: couldn't convert input to numbers" << endl;
				}
			} else {
				cout << "kill - [args] - 1. TARGET INDEX | 2. FRAME INDEX " << endl;
			}
		} else {
			cout << "kill - [args] - 1. TARGET INDEX | 2. FRAME INDEX" << endl;
		}
	} else if (!strcmp(splitString.at(0).c_str(), "players"))
	{
		try {
			if (splitString.at(1).length() > 0) {
				listPlayersConsole(splitString.at(1));
			} else {
				listPlayersConsole("");
			}
		} catch (std::exception e)
		{
			listPlayersConsole("");
		}
	}
}