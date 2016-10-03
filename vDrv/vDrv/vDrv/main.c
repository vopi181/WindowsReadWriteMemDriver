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

NTSTATUS copyMemory( PRWM vrwm )
{
	PEPROCESS					targetProc			= NULL;
	NTSTATUS status;

	status = PsLookupProcessByProcessId((HANDLE)vrwm->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		SIZE_T bytes;
		if (vrwm->write && vrwm->dAddress)
		{
			if (vrwm->topPtr)
				status = MmCopyVirtualMemory(PsGetCurrentProcess(), (PVOID)vrwm->topPtr, targetProc, (PVOID)vrwm->dAddress, vrwm->size, KernelMode, &bytes);
		} else {
			status = MmCopyVirtualMemory(targetProc, (PVOID)vrwm->dAddress, PsGetCurrentProcess(), (PVOID)vrwm->lowPtr, vrwm->size, KernelMode, &bytes);
		}
	}

	if (targetProc)
		ObDereferenceObject(targetProc);

	return status;
}

NTSTATUS getHandle(PHEADER vheader)
{
	NTSTATUS status;
	PEPROCESS					targetProc = NULL;
	PVOID						value;

	status = PsLookupProcessByProcessId((HANDLE)vheader->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		KAPC_STATE	apc;
		KeStackAttachProcess(targetProc, &apc);

		value				= PsGetProcessSectionBaseAddress( targetProc );
		vheader->imageBase	= (ULONG)value;

		KeUnstackDetachProcess( &apc );
	}

	return status;
}

NTSTATUS allocateMemory( PALLOC valloc, PALLOCRESULT result )
{
	NTSTATUS status;
	PEPROCESS					targetProc			= NULL;

	status = PsLookupProcessByProcessId((HANDLE)valloc->pID, &targetProc);
	if (NT_SUCCESS(status))
	{
		KAPC_STATE	apc;
		ULONG_PTR	size		= valloc->size;
		PVOID		dAddress	= (PVOID)valloc->dAddress;

		KeStackAttachProcess( targetProc, &apc );

		if (valloc->allocate) {
			
			status = ZwAllocateVirtualMemory( ZwCurrentProcess(), &dAddress, 0, &size, valloc->type, valloc->protection );
			result->dAddress = (ULONGLONG)dAddress;
			result->size = size;

		} else {
			status = ZwFreeVirtualMemory(ZwCurrentProcess(), &dAddress, &size, valloc->type);
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

			case IOCTL_vHANDLE:
			{
				bytesIO = 0;
				if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(vHEADER)) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				// GET PROCESS HANDLE
				status = getHandle( (PHEADER)pBuf );
				bytesIO = sizeof( vHEADER );

				break;
			}
			case IOCTL_vRWM:
			{
				bytesIO = 0;
				if ( (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(vRWM) ) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}
				
				// FUNCTION THAT COPIES MEMORY
				status = copyMemory( (PRWM)pBuf );
				break;
			}
			case IOCTL_vALLOC:
			{
				vALLOCRESULT result;

				bytesIO = 0;
				if ((pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength) != (unsigned long)sizeof(vALLOC)) { // 32x PTR to 64x PTR
					status = STATUS_INVALID_PARAMETER;
					break;
				}
				if ( outputBufferLength < sizeof(vALLOCRESULT) ) {
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				status = allocateMemory( (PALLOC)pBuf, &result );
				if (NT_SUCCESS(status)) {
					bytesIO = sizeof(vALLOCRESULT);
					RtlCopyMemory( pBuf, &result, sizeof(vALLOCRESULT) );
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

	const WCHAR driverName[] = L"\\Driver\\vDrv";

	UNICODE_STRING  drvName;

	RtlInitUnicodeString(&drvName, driverName);
	NTSTATUS status = IoCreateDriver(&drvName, &DriverInitialize);

	return status;
}