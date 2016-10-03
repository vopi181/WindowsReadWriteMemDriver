#include "driverManager.h"

/*
bool driverManager::disableDSE()
{
	GUID		gidReference;
	HRESULT		hCreateGuid				= CoCreateGuid(&gidReference);
	TCHAR		tempPath[MAX_PATH]	= { 0 };

	LPOLESTR bstrGuid;
	StringFromCLSID( gidReference, &bstrGuid );
	GetTempPath( MAX_PATH, tempPath );

	if( bstrGuid == nullptr )
		return false;

	this->filePath = wstring( tempPath ) + wstring( bstrGuid ) + L".exe";
	this->fileHandle = CreateFile( filePath.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( fileHandle != INVALID_HANDLE_VALUE ) {
		CloseHandle( this->fileHandle );

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof( si ) );
		si.cb = sizeof( si );
		ZeroMemory( &pi, sizeof( pi ) );

		DWORD exitCode = 10;
		if (CreateProcess(this->filePath.c_str(), NULL, NULL, NULL,
			false, NULL, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject( pi.hProcess, INFINITE );

			if( !GetExitCodeProcess( pi.hProcess, &exitCode ) ) 
				return false;

			if( !exitCode )
				return true;

		} else { CoTaskMemFree(bstrGuid); return false; }

	} else { CoTaskMemFree(bstrGuid); return false; }

	CoTaskMemFree(bstrGuid);
	return false;
}

bool driverManager::enableDSE()
{
	if (filePath.length() < 1 )
		return false;

	fileHandle = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fileHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(this->fileHandle);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		wstring startArgs	= filePath + L" -d\0";
		WCHAR*  sArgs		= new WCHAR[ startArgs.length() ]; // memory-leak - need to fix later
		wcscpy_s( sArgs, startArgs.length(), startArgs.c_str() );

		DWORD exitCode = 10;
		if (CreateProcess(this->filePath.c_str(), sArgs, NULL, NULL,
			false, NULL, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);

			if (!GetExitCodeProcess(pi.hProcess, &exitCode))
				return false;

			if (!exitCode)
				return true;

		} else { return false; }

	} else { return false; }

	return false;
}

bool driverManager::loadDriver( wstring path, wstring serviceName )
{
	driverPath = path;
	driverName = serviceName;

	SC_HANDLE scHandle = OpenSCManager( nullptr, nullptr, SC_MANAGER_ALL_ACCESS );
	if (scHandle != INVALID_HANDLE_VALUE)
	{
		SC_HANDLE sHandle = CreateService( scHandle, serviceName.c_str(), serviceName.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, 
										   SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, path.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr );

		if (sHandle != nullptr) {
			StartService( sHandle, NULL, NULL );

			CloseServiceHandle(scHandle);
			CloseServiceHandle(sHandle);

			return true;
		} else {
			if (GetLastError() == ERROR_SERVICE_EXISTS) {
				SERVICE_STATUS ss;
				ControlService( sHandle, SERVICE_CONTROL_STOP, &ss );
				StartService( sHandle, NULL, NULL );

				CloseServiceHandle( scHandle );
				CloseServiceHandle( sHandle );

				return true;
			}
		}
	}


	CloseServiceHandle( scHandle );
	return false;
}

bool driverManager::unloadDriver()
{
	SC_HANDLE scManager = OpenSCManager( nullptr, nullptr, SC_MANAGER_ALL_ACCESS );
	if (scManager != INVALID_HANDLE_VALUE)
	{
		SC_HANDLE sHandle = OpenService( scManager, driverName.c_str(), SERVICE_ALL_ACCESS );
		if (sHandle != INVALID_HANDLE_VALUE)
		{
			SERVICE_STATUS ss;
			ControlService( sHandle, SERVICE_CONTROL_STOP, &ss );
			DeleteService( sHandle );

			CloseServiceHandle( scManager );
			CloseServiceHandle( sHandle );

			return true;
		}
	}

	CloseServiceHandle(scManager);
	return false;
}
*/
bool driverManager::loadDriverless(wstring relativePath, wstring ldrName, wstring drvName)
{
	if( PathFileExists( wstring( relativePath.c_str() + ldrName ).c_str() ) && drvName.length() > 0 )
	{
		if ( (INT)ShellExecute( NULL, L"open", wstring(relativePath.c_str() + ldrName).c_str(), drvName.c_str(), relativePath.c_str(), 0 ) > 32 ) {
			return true;
		}
	}

	return false;
}

