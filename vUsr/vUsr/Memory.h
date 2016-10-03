#ifndef HEADER_MEMORY
#define HEADER_MEMORY

#include <Windows.h>
#include <Tlhelp32.h>
#include <string>
#include <mutex>


#include "IOCTL.h"

using namespace std;

// ALLOCATING MEMORY TYPE
#define BUFFERED_MEMORY 1;

enum ERROR_MEM
{
	ERROR_DRIVER,
	ERROR_ATTACH,
	ERROR_INVALID_MEMORY,
	ERROR_INVALID_SIZE,
	ERROR_NULL
};

class Memory
{
public:
	 Memory();
	 ~Memory();

	VOID Detach();
	VOID Attach( DWORD procID );

	VOID deallocateMemory(DWORD dwAddress, SIZE_T size);
	DWORD allocateMemory( SIZE_T size );

	VOID clearMemory();
	VOID clearFile();

	bool isAttached()
	{
		return (this->hDevice != NULL && this->procID != 0 );
	}
	bool isRunning( );

	template <typename T>
	T		read( DWORD dAddress );

	template <typename T>
	VOID	write( DWORD dAddress, T value, ULONG size );

	template <typename T>
	VOID	write(DWORD dAddress, T value );

	template <typename T>
	T		readPointer( DWORD dAddress, DWORD offsets[], int offsetCount );
	DWORD	readPointer( DWORD dOffsets[], int offsetCount );

	std::string readString( DWORD dAddress, SIZE_T size );
	std::string readStringUnformated( DWORD dAddress, SIZE_T size );

	DWORD getProcessIdFromName( char* name );
	DWORD getProcID() { return this->procID; }

private:
	char* readStr( DWORD dAddress, ULONG size );

	HANDLE	hDevice			= NULL;
	DWORD	procID			= NULL;
};

#endif