#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>


#pragma warning( disable : 4201 4311 )

#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x4550

NTKERNELAPI
NTSTATUS
IoCreateDriver(
	IN PUNICODE_STRING DriverName, OPTIONAL
	IN PDRIVER_INITIALIZE InitializationFunction
);

NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
	IN PEPROCESS P1,
	IN PVOID A1,
	IN PEPROCESS P2,
	OUT PVOID A2,
	IN SIZE_T BSize,
	IN KPROCESSOR_MODE Mode,
	OUT PSIZE_T NumberOfBytesCopied
);


NTKERNELAPI 
PVOID 
PsGetProcessSectionBaseAddress( IN PEPROCESS peProcess );

NTKERNELAPI
PPEB
PsGetProcessPeb(IN PEPROCESS peProcess);


_Dispatch_type_(IRP_MJ_CREATE);
DRIVER_DISPATCH  Function_IRP_MJ_CREATE;
_Dispatch_type_(IRP_MJ_CLOSE);
DRIVER_DISPATCH  Function_IRP_MJ_CLOSE;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL);
DRIVER_DISPATCH  Function_IRP_DEVICE_CONTROL;

DRIVER_INITIALIZE DriverInitialize;
DRIVER_INITIALIZE DriverEntry;
#pragma alloc_text(INIT, DriverEntry)

const WCHAR deviceNameBuffer[] = L"\\Device\\FMGR";
const WCHAR deviceSymLinkBuffer[] = L"\\DosDevices\\FMGR";


#define IOCTL_INJECT CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )
#define IOCTL_HANDLE CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )
#define IOCTL_RWM CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )
#define IOCTL_ALLOC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )

typedef struct _INJECT {
	ULONG		pID;
	ULONG		imageBase;
} INJECT, *PINJECT;

typedef struct _HEADER {
	ULONG		pID;
	ULONG		imageBase;
} HEADER, *PHEADER;

typedef struct _RWM {
	ULONG		pID;
	ULONG		size;
	ULONG		dAddress;
	BOOLEAN		write;
	ULONG		topPtr; //BUFFER
	ULONG		lowPtr; //RETVALUE
} RWM, *PRWM;

typedef struct _ALLOC {
	ULONG		pID;
	ULONG		size;
	ULONGLONG	dAddress;
	BOOLEAN		allocate;

	ULONG		type;
	ULONG		protection;
} ALLOC, *PALLOC;

typedef struct _ALLOCRESULT {
	ULONGLONG	dAddress;
	ULONGLONG	size;
} ALLOCRESULT, *PALLOCRESULT;