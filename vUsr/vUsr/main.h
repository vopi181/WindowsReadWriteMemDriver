#pragma once

#include <Dwmapi.h> 
#include <TlHelp32.h>
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>


// MY HEADERS

#include "Memory.h"
#include "driverManager.h"
#include "Logger.h"

#pragma comment(lib,"Dwmapi.lib")

extern bool				displayEmptyCars;
extern bool				displayPlayers;
extern bool				displayCars;
extern bool				displayItems;

// GLOBAL VARIABLES

extern Memory*			m;

extern driverManager	dMGR;
extern Logger			logger;
extern FileLogger		fileLogger;

