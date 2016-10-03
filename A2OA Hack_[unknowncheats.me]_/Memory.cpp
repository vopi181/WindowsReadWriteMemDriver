#include "Memory.h"

extern d3Console*		console;

Memory::Memory()
{
}

Memory::~Memory()
{
	CloseHandle( hDevice );
}

VOID Memory::Attach( DWORD procID )
{
	this->procID	= procID;

	this->hDevice	= CreateFile(TEXT("\\\\.\\FMGR"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

VOID Memory::deallocateMemory(DWORD dwAddress, SIZE_T size)
{
	return VOID();
}

DWORD Memory::allocateMemory(SIZE_T size)
{
	ALLOC			memory;
	ALLOCRESULT		result;

	DWORD dwBytesRead;

	memory.size			= size;
	memory.dAddress		= NULL;
	memory.allocate		= 1;
	memory.pID			= this->procID;
	memory.type			= MEM_COMMIT;
	memory.protection	= PAGE_EXECUTE_READWRITE;

	if (!DeviceIoControl(hDevice, IOCTL_ALLOC, &memory, sizeof(ALLOC), &result, sizeof(ALLOCRESULT), &dwBytesRead, NULL))
	{
		throw ERROR_DRIVER;
	}

	if (dwBytesRead != sizeof(ALLOCRESULT))
	{
		throw ERROR_INVALID_MEMORY;
	}

	return (DWORD)result.dAddress;
}

VOID Memory::clearMemory()
{
	return VOID();
}

VOID Memory::clearFile()
{
	return VOID();
}

VOID Memory::Detach( )
{
	if( !isAttached() )
		return;

	CloseHandle( hDevice );
	return;
}

/*

		PROCESS FUNCTIONS

*/

bool Memory::isRunning( )
{
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = NULL;

	bool retVal;

	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	pe32.dwSize = sizeof( pe32 );

	if( hSnapshot == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	retVal = Process32First( hSnapshot, &pe32 ) != 0;

	while( retVal == 1 )
	{
		if( this->procID == pe32.th32ProcessID ) // IF 0 THEN 1STR == 2STR.
		{
			CloseHandle( hSnapshot );
			return true;
		}

		retVal = Process32Next( hSnapshot, &pe32 ) != 0;
	}

	return false;
}

DWORD Memory::getProcessIdFromName( char* procName )
{
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = NULL;

	bool retVal;

	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	pe32.dwSize = sizeof( pe32 );

	if( hSnapshot == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	retVal = Process32First( hSnapshot, &pe32 ) != 0;

	while( retVal )
	{
		char* exe = new char[ MAX_PATH ];
		wcstombs_s( NULL, exe, MAX_PATH, pe32.szExeFile, MAX_PATH );

		if( !strcmp( procName, exe ) ) // IF 0 THEN 1STR == 2STR.
		{
			CloseHandle( hSnapshot );
			return pe32.th32ProcessID;
		}

		retVal = Process32Next( hSnapshot, &pe32 ) != 0;
	}

	return 0;
}



/*

		RPM FUNCTIONS

*/

template <typename T>
T Memory::read( DWORD dAddress )
{
	if (!isAttached()) {
		throw ERROR_ATTACH;
	}
		
	if (!dAddress || !procID) {
		throw ERROR_INVALID_MEMORY;
	}	

	DWORD	dwBytesRead		= 0;
	RWM		mem;

	mem.pID			= this->procID;
	mem.retValue	= new T;
	mem.size		= sizeof( T );
	mem.dAddress	= dAddress;
	mem.write		= 0;

	if( !DeviceIoControl( hDevice, IOCTL_RPM, &mem, sizeof( RWM ), NULL, NULL, &dwBytesRead, NULL ) )
	{
		throw ERROR_DRIVER;
	}	
	if (!mem.retValue)
	{
		delete mem.retValue;
		throw ERROR_INVALID_MEMORY;
	}
	
	T	data = *(T*)mem.retValue; // casting memory so leaks dont happen
	delete mem.retValue;

	return data;
}

// MEMORY LEAK AT THE BOTTOM OF FNC
char* Memory::readStr( DWORD dAddress, ULONG size )
{
	if (!isAttached()) {
		throw NULL;
	}

	if ( !dAddress || !procID || !size ) {
		return "";
	}
		

	RWM		mem;
	DWORD	dwBytesRead		= 0;

	mem.pID			= this->procID;
	mem.retValue	= new char[size];
	mem.size		= size;
	mem.dAddress	= dAddress;
	mem.write		= 0;

	if( !DeviceIoControl( hDevice, IOCTL_RPM, &mem, sizeof( RWM ), NULL, NULL, &dwBytesRead, NULL ) )
	{
		delete[] mem.retValue;
		return "";
	}

	return (char*)mem.retValue;
}

template <typename T>
T Memory::readPointer( DWORD dAddress, DWORD offsets[], int offsetCount )
{
	if( !dAddress || !offsets || offsetCount < 1 )
		throw ERROR_INVALID_MEMORY;

	DWORD	address		= 0;
	T		data		   ;

	try{
		address = this->read<int>( dAddress );
	} catch( ERROR_MEM e )
	{
		throw e;
	}
		
	for( int i = 0; i < offsetCount; i++ )
	{
		if( i == (offsetCount - 1) )
		{
			try{
				data = this->read<T>( address + offsets[ i ] );
			} catch( ERROR_MEM e )
			{
				throw e;
			}
				

			break;
		}

		if( !address )
			throw ERROR_INVALID_MEMORY;

		address += offsets[ i ];
		try{
			address = read<int>( address );
		} catch( ERROR_MEM e )
		{
			throw e;
		}
			
		
	}


	return data;
}

DWORD Memory::readPointer( DWORD dOffsets[], int offsetCount )
{
	if( offsetCount < 1 || offsetCount > 10 )
		throw ERROR_INVALID_SIZE;

	DWORD finalAddress = dOffsets[ 0 ];

	for( int i = 1; i < ( offsetCount + 1 ); i++ )
	{
		if (!finalAddress)
			throw ERROR_NULL;

		try{
			finalAddress = this->read<DWORD>( finalAddress );
		} catch( ERROR_MEM e )
		{
			throw e;
		}

		if( i == offsetCount )
			break;

		finalAddress += dOffsets[ i ];
	}

	return finalAddress;
}

/*

		WPM FUNCTIONS

*/

template <typename T>
VOID Memory::write( DWORD dAddress, T value, ULONG size )
{

	RWM		mem;
	DWORD	dwBytesRead	= 0;

	mem.pID			= this->procID;
	mem.dAddress	= dAddress;
	mem.buffer		= &value;
	mem.size		= size;
	mem.write		= 1;

	if (!isAttached()) {
		throw ERROR_ATTACH;
	}

	if (!dAddress || !procID) {
		throw ERROR_INVALID_MEMORY;
	}
		
	DeviceIoControl( hDevice, IOCTL_RPM, &mem, sizeof( RWM ), NULL, NULL, &dwBytesRead, NULL );
}

template<typename T>
VOID Memory::write(DWORD dAddress, T value)
{

	RWM		mem;
	DWORD	dwBytesRead = 0;

	mem.pID			= this->procID;
	mem.dAddress	= dAddress;
	mem.buffer		= &value;
	mem.size		= sizeof(T);
	mem.write		= 1;

	if (!isAttached()) {
		throw ERROR_ATTACH;
	}
		

	if (!dAddress || !procID) {
			throw ERROR_INVALID_MEMORY;
	}
		

	DeviceIoControl(hDevice, IOCTL_RPM, &mem, sizeof(RWM), NULL, NULL, &dwBytesRead, NULL);
}

/*

		WRAPPER FUNCTIONS

*/

std::string Memory::readString( DWORD dAddress, SIZE_T size )
{
	if( size > 200 )
		return "";

	char* buffer = NULL;
	string text;

	buffer = this->readStr( dAddress, size );
	if( !buffer )
		return "";

	for( SIZE_T i = 0; i < strlen( buffer ); i++ )
	{
		if( buffer[ i ] == '\0' )
			return text;
		if( buffer[ i ] == ' ' && buffer[ i + 1 ] == ' ' )
			return text;

		text += buffer[ i ];
	}

	delete[] buffer;
	return text;
}

std::string Memory::readStringUnformated(DWORD dAddress, SIZE_T size)
{
	char* buffer = NULL;
	string text;

	buffer = this->readStr(dAddress, size);
	if (!buffer)
		return "";

	for (SIZE_T i = 0; i < strlen(buffer); i++)
	{
		text += buffer[i];
	}

	delete[] buffer;
	return text;
}



void tempFunc()
{
	Memory m;

	m.write<BYTE>( NULL, NULL );
	m.write<float>( NULL, NULL );
	m.write<INT>( NULL, NULL );
	m.write<DWORD>( NULL, NULL );
	m.write<D3DXVECTOR3>(NULL,D3DXVECTOR3(0,0,0));

	m.write<char*>( NULL, NULL, NULL );

	m.read<BYTE>				( NULL );
	m.read<int>					( NULL );
	m.read<float>				( NULL );
	m.read<DWORD>				( NULL );
	m.read<D3DXVECTOR3>			( NULL );

	m.readPointer<D3DXVECTOR3>	( NULL, NULL, NULL );
	m.readPointer<DWORD>		( NULL, NULL, NULL );
	m.readPointer<int>			( NULL, NULL, NULL );
	m.readPointer<float>		( NULL, NULL, NULL );
}