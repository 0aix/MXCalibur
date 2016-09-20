#pragma once

#include <ntddk.h>
#include <kbdmou.h>
#include <ntddmou.h>
#include <wdf.h>

typedef struct _DEVICE_EXTENSION
{
	CONNECT_DATA UpperConnectData;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, FilterGetData);

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD MouFilter_EvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL MouFilter_EvtIoInternalDeviceControl;

VOID MouFilter_DispatchPassThrough(WDFREQUEST Request, WDFIOTARGET Target);

VOID MouFilter_ServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);