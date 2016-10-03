#include "main.h"


NTSTATUS Function_IRP_MJ_CREATE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS Function_IRP_MJ_CLOSE(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS copyMemory( PRWM rwm )
{
	PEPROCESS					targetProc			= NULL;
	NTSTATUS status;

	status = PsLookupProcessByProcessId((HANDLE)rwm->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		SIZE_T bytes;
		if (rwm->write && rwm->dAddress)
		{
			if (rwm->topPtr)
				status = MmCopyVirtualMemory(PsGetCurrentProcess(), (PVOID)rwm->topPtr, targetProc, (PVOID)rwm->dAddress, rwm->size, KernelMode, &bytes);
		} else {
			status = MmCopyVirtualMemory(targetProc, (PVOID)rwm->dAddress, PsGetCurrentProcess(), (PVOID)rwm->lowPtr, rwm->size, KernelMode, &bytes);
		}
	}

	if (targetProc)
		ObDereferenceObject(targetProc);

	return status;
}

NTSTATUS getHandle(PHEADER header)
{
	NTSTATUS status;
	PEPROCESS					targetProc = NULL;
	PVOID						value;

	status = PsLookupProcessByProcessId((HANDLE)header->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		KAPC_STATE	apc;
		KeStackAttachProcess(targetProc, &apc);

		value				= PsGetProcessSectionBaseAddress( targetProc );
		header->imageBase	= (ULONG)value;

		KeUnstackDetachProcess( &apc );
	}

	return status;
}

NTSTATUS allocateMemory( PALLOC alloc, PALLOCRESULT result )
{
	NTSTATUS status;
	PEPROCESS					targetProc			= NULL;

	status = PsLookupProcessByProcessId((HANDLE)alloc->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		KAPC_STATE	apc;
		ULONG_PTR	size		= alloc->size;
		PVOID		dAddress	= (PVOID)alloc->dAddress;

		KeStackAttachProcess( targetProc, &apc );

		if (alloc->allocate) {
			
			status = ZwAllocateVirtualMemory( ZwCurrentProcess(), &dAddress, 0, &size, alloc->type, alloc->protection );
			result->dAddress = (ULONGLONG)dAddress;
			result->size = size;

		} else {
			status = ZwFreeVirtualMemory(ZwCurrentProcess(), &dAddress, &size, alloc->type);
		}

		KeUnstackDetachProcess( &apc );
	}

	return status;
}

NTSTATUS Function_IRP_DEVICE_CONTROL(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER( pDeviceObject );

	PIO_STACK_LOCATION			pIoStackLocation;
	NTSTATUS					status				= STATUS_SUCCESS;
	PVOID						pBuf				= Irp->AssociatedIrp.SystemBuffer;
	ULONG						bytesIO = 0;
	ULONG						outputBufferLength;

	pIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	outputBufferLength = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	do {

		if ( pIoStackLocation == NULL ) {
			status = STATUS_INTERNAL_ERROR;
			break;
		}

		pBuf = Irp->AssociatedIrp.SystemBuffer;
		if (pBuf == NULL) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode) {

			case IOCTL_HANDLE:
			{
				bytesIO = 0;
				if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(HEADER)) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				// GET PROCESS HANDLE
				status = getHandle( (PHEADER)pBuf );
				bytesIO = sizeof( HEADER );

				break;
			}
			case IOCTL_INJECT:
			{
				// NOT COMPLETED
			}
			case IOCTL_RWM:
			{
				bytesIO = 0;
				if ( (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(RWM) ) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}
				
				// FUNCTION THAT COPIES MEMORY
				status = copyMemory( (PRWM)pBuf );
				break;
			}
			case IOCTL_ALLOC:
			{
				ALLOCRESULT result;

				bytesIO = 0;
				if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(ALLOC)) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}
				if ( outputBufferLength < sizeof(ALLOCRESULT) ) {
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				status = allocateMemory( (PALLOC)pBuf, &result );
				if (NT_SUCCESS(status)) {
					bytesIO = sizeof(ALLOCRESULT);
					RtlCopyMemory( pBuf, &result, sizeof(ALLOCRESULT) );
				}

				break;
			}
			default:
			{
				status = STATUS_INVALID_PARAMETER;
			}
		}

	} while (FALSE);

	Irp->IoStatus.Status		= status;
	Irp->IoStatus.Information	= bytesIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}


NTSTATUS DriverInitialize(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING						deviceNameUnicodeString, deviceSymLinkUnicodeString;
	NTSTATUS							status;

	PDEVICE_OBJECT						devobj;

	// INITILISING STRINGS INSIDE THE DRIVER
	RtlInitUnicodeString(&deviceNameUnicodeString, deviceNameBuffer);
	RtlInitUnicodeString(&deviceSymLinkUnicodeString, deviceSymLinkBuffer);

	status = IoCreateDevice(DriverObject,
		0,
		&deviceNameUnicodeString,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&devobj);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	devobj->Flags |= DO_BUFFERED_IO;

	status = IoCreateSymbolicLink(&deviceSymLinkUnicodeString, &deviceNameUnicodeString);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = &Function_IRP_MJ_CREATE;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = &Function_IRP_MJ_CLOSE;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &Function_IRP_DEVICE_CONTROL;
	DriverObject->DriverUnload = NULL;

	devobj->Flags &= ~DO_DEVICE_INITIALIZING;

	return status;
}

NTSTATUS DriverEntry(
	_In_  struct _DRIVER_OBJECT *DriverObject,
	_In_  PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	const WCHAR driverName[] = L"\\Driver\\FMGR";

	UNICODE_STRING  drvName;

	RtlInitUnicodeString(&drvName, driverName);
	NTSTATUS status = IoCreateDriver(&drvName, &DriverInitialize);

	return status;
}