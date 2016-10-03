#pragma once

#include "Shlwapi.h"
#include <string>
#include <windows.h>

#pragma comment(lib,"Shlwapi.lib")

using namespace std;

// CONSTANTS (NEED TO COPY IN VBOX DRV)
static BYTE		VBoxDriver[] = {0x3,0x4};

class driverManager {
public:

	// DSE
	/*
	bool disableDSE();
	bool enableDSE();

	bool loadDriver( wstring path, wstring serviceName );
	bool unloadDriver();
	*/

	bool loadDriverless(wstring relativePath, wstring ldrName, wstring drvName);

private:

	wstring		driverPath;
	wstring		driverName;


	wstring		filePath;
	HANDLE		fileHandle;
};