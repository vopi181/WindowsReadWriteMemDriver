#pragma once

#include <Dwmapi.h> 
#include <TlHelp32.h>
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>


// MY HEADERS
#include "renderManager.h"
#include "d3Console.h"
#include "Memory.h"
#include "driverManager.h"
#include "D3D9_Menu.h"
#include "Logger.h"

#pragma comment(lib,"Dwmapi.lib")

extern bool				displayEmptyCars;
extern bool				displayPlayers;
extern bool				displayCars;
extern bool				displayItems;

// GLOBAL VARIABLES
extern rManager*		d3;
extern d3Console*		console;
extern Memory*			m;
extern D3D9Menu			d3d9;
extern driverManager	dMGR;
extern Logger			logger;
extern FileLogger		fileLogger;

extern LPD3DXFONT		font;