#include "Sovereign.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, MouFilter_EvtDeviceAdd)
#pragma alloc_text (PAGE, MouFilter_EvtIoInternalDeviceControl)
#endif

#pragma warning(push)
#pragma warning(disable:4055) // type case from PVOID to PSERVICE_CALLBACK_ROUTINE
#pragma warning(disable:4152) // function/data pointer conversion in expression

NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath)
{
	WDF_DRIVER_CONFIG config;
	NTSTATUS status;

	WDF_DRIVER_CONFIG_INIT(&config, MouFilter_EvtDeviceAdd);

	status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);

	return status;
}

NTSTATUS MouFilter_EvtDeviceAdd(WDFDRIVER Driver, PWDFDEVICE_INIT DeviceInit)
{
	WDF_OBJECT_ATTRIBUTES deviceAttributes;
	NTSTATUS status;
	WDFDEVICE hDevice;
	WDF_IO_QUEUE_CONFIG ioQueueConfig;

	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	WdfFdoInitSetFilter(DeviceInit);

	WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_MOUSE);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_EXTENSION);

	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &hDevice);
	if (!NT_SUCCESS(status))
		return status;

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);

	ioQueueConfig.EvtIoInternalDeviceControl = MouFilter_EvtIoInternalDeviceControl;

	status = WdfIoQueueCreate(hDevice, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);

	return status;
}

VOID MouFilter_DispatchPassThrough(WDFREQUEST Request, WDFIOTARGET Target)
{
	WDF_REQUEST_SEND_OPTIONS options;
	BOOLEAN ret;
	NTSTATUS status = STATUS_SUCCESS;

	WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

	ret = WdfRequestSend(Request, Target, &options);

	if (ret == FALSE)
	{
		status = WdfRequestGetStatus(Request);
		WdfRequestComplete(Request, status);
	}
}

VOID MouFilter_EvtIoInternalDeviceControl(WDFQUEUE Queue, WDFREQUEST Request, size_t OutputBufferLength, size_t InputBufferLength, ULONG IoControlCode)
{
	PDEVICE_EXTENSION devExt;
	PCONNECT_DATA connectData;
	NTSTATUS status = STATUS_SUCCESS;
	WDFDEVICE hDevice;
	size_t length;

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	PAGED_CODE();

	hDevice = WdfIoQueueGetDevice(Queue);
	devExt = FilterGetData(hDevice);

	switch (IoControlCode)
	{
	case IOCTL_INTERNAL_MOUSE_CONNECT:
		if (devExt->UpperConnectData.ClassService != NULL)
		{
			status = STATUS_SHARING_VIOLATION;
			break;
		}

		status = WdfRequestRetrieveInputBuffer(Request, sizeof(CONNECT_DATA), &connectData, &length);
		if (!NT_SUCCESS(status)) 
			break;

		devExt->UpperConnectData = *connectData;

		connectData->ClassDeviceObject = WdfDeviceWdmGetDeviceObject(hDevice);
		connectData->ClassService = MouFilter_ServiceCallback;
		break;
	case IOCTL_INTERNAL_MOUSE_DISCONNECT:
		status = STATUS_NOT_IMPLEMENTED;
		break;
	case IOCTL_MOUSE_QUERY_ATTRIBUTES:
	default:
		break;
	}

	if (!NT_SUCCESS(status))
	{
		WdfRequestComplete(Request, status);
		return;
	}

	MouFilter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(hDevice));
}

VOID MouFilter_ServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed)
{
	PDEVICE_EXTENSION devExt;
	WDFDEVICE hDevice;
	
	for (PMOUSE_INPUT_DATA input = InputDataStart; input != InputDataEnd; input++)
	{
		if (input->Flags ^ MOUSE_MOVE_ABSOLUTE)
		{
			input->LastX *= -1;
			input->LastY *= -1;
		}
	}

	hDevice = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);
	devExt = FilterGetData(hDevice);

	(*(PSERVICE_CALLBACK_ROUTINE)devExt->UpperConnectData.ClassService)(devExt->UpperConnectData.ClassDeviceObject, InputDataStart, InputDataEnd, InputDataConsumed);
}

#pragma warning(pop)