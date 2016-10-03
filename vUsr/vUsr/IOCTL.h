//### IOCTL declaration ###
#pragma once

#define IOCTL_RPM CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )
#define IOCTL_ALLOC CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS )

typedef struct _RWM {
	ULONG		pID;
	ULONG		size;
	ULONG		dAddress;
	UCHAR		write;

	PVOID		buffer;
	PVOID		retValue;
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