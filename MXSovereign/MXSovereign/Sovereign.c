#include "Sovereign.h"
#include "math.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, MouFilter_EvtDeviceAdd)
#pragma alloc_text (PAGE, MouFilter_EvtIoInternalDeviceControl)
#endif

#pragma warning(push)
#pragma warning(disable:4055) // type case from PVOID to PSERVICE_CALLBACK_ROUTINE
#pragma warning(disable:4152) // function/data pointer conversion in expression

extern int _fltused = 0x9875;

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

typedef struct
{
	double upper;
	double x;
	double intercept;
	double slope;
} InfPoint;

InfPoint infpts[] =
{
	{ 0.4300079345703125, 0.0, 0.0, 2.488946453284127603704623682623 },
	{ 1.25, 0.4300079345703125, 1.0702667236328125, 3.7443755931446435549600848545749 },
	{ 3.8600006103515625, 1.25, 4.140625, 5.6872592064262287414717420154459 },
	{ 10000.0, 3.8600006103515625, 18.984375, 11.753337912940458211225723261969 }
};

double MouseToPointer(double v)
{
	double absv = fabs(v) / 3.5;

	for (int i = 0; i < sizeof(infpts); i++)
		if (absv <= infpts[i].upper)
			return (absv - infpts[i].x) * infpts[i].slope + infpts[i].intercept;

	return 0.0;
}

LONG round(double d)
{
	return d >= 0.0 ? (LONG)(d + 0.5) : (LONG)(d - 0.5);
}

int overwatch = 0;
double ax = 65535.0 / 2.0;
double ay = 65535.0 / 2.0;
double lx = 0.0;
double ly = 0.0;

VOID MouFilter_ServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed)
{
	PDEVICE_EXTENSION devExt;
	WDFDEVICE hDevice;

	KFLOATING_SAVE kfsave;
	NTSTATUS status = KeSaveFloatingPointState(&kfsave);

	if (!NT_SUCCESS(status))
		return;

	hDevice = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);
	devExt = FilterGetData(hDevice);

	for (PMOUSE_INPUT_DATA input = InputDataStart; input != InputDataEnd; input++)
	{
		if (input->Flags & MOUSE_ATTRIBUTES_CHANGED)
			continue;

		// THE DEVICE WRITES TO AN INPUT BUFFER WHICH IS CREATED WHEN THE DEVICE CONNECTS.
		// IT DOES NOT NECESSARILY OVERWRITE FLAGS ON EVERY UPDATE.
		// THIS MEANS IF WE SET THE FLAG TO ABSOLUTE, IT MIGHT NOT CHANGE BACK TO RELATIVE.
		// RIGHT NOW, ALL DATA IS ASSUMED TO BE GIVEN AS RELATIVE. IN THE FUTURE, 

		if (input->ButtonFlags & MOUSE_BUTTON_5_DOWN)
		{
			overwatch = !overwatch;
			input->ButtonFlags &= ~MOUSE_BUTTON_5_DOWN;
		}
		else if (input->ButtonFlags & MOUSE_BUTTON_5_UP)
			input->ButtonFlags &= ~MOUSE_BUTTON_5_UP;

		// new
		if (overwatch)
		{
			LONG dx = input->LastX;
			LONG dy = input->LastY;

			const double x_sensitivity = 0.286111;
			const double y_sensitivity = 0.195922;
			double qx = x_sensitivity * dx * 65535.0 / 1366.0 + lx;
			double qy = y_sensitivity * dy * 65535.0 / 768.0 + ly;
			LONG rx = round(qx);
			LONG ry = round(qy);
			lx = qx - rx;
			ly = qy - ry;
			input->LastX = rx;
			input->LastY = ry;
		}

#define ERASE_FOR_NOW
#ifndef ERASE_FOR_NOW

		if (dx != 0 || dy != 0)
		{
			const double sens = 0.2 * 0.8;
			const double rev_sens = -1.0 * sens;

			double DX = 0.0;
			double DY = 0.0;

			if (dy == 0.0)
			{
				if (dx > 0.0)
					DX = sens * MouseToPointer(dx) * 65535.0 / 1366.0;
				else
					DX = rev_sens * MouseToPointer(dx) * 65535.0 / 1366.0;
			}
			else if (dx == 0.0)
			{
				if (dy > 0.0)
					DY = sens * MouseToPointer(dy) * 65535.0 / 768.0;
				else
					DY = rev_sens * MouseToPointer(dy) * 65535.0 / 768.0;
			}
			else
			{
				double v = sqrt(dx * dx + dy * dy);
				double av = sens * MouseToPointer(v);
				DX = dx / v * av * 65535.0 / 1366.0;
				DY = dy / v * av * 65535.0 / 768.0;
			}

			if (!overwatch)
			{
				ax += DX;
				ay += DY;
				ax = max(0.0, min(ax, 65535.0));
				ay = max(0.0, min(ay, 65535.0));

				input->LastX = round(ax);
				input->LastY = round(ay);
				input->Flags |= MOUSE_MOVE_ABSOLUTE;
			}
			else
			{
				const double x_sensitivity = 0.286111;
				const double y_sensitivity = 0.195922;
				double qx = x_sensitivity * DX - dx + lx;
				double qy = y_sensitivity * DY - dy + ly;
				LONG rx = round(qx);
				LONG ry = round(qy);
				lx = qx - rx;
				ly = qy - ry;
				input->LastX = rx;
				input->LastY = ry;
				input->Flags &= ~MOUSE_MOVE_ABSOLUTE;
			}
		}
#endif
	}

	KeRestoreFloatingPointState(&kfsave);

	(*(PSERVICE_CALLBACK_ROUTINE)devExt->UpperConnectData.ClassService)(devExt->UpperConnectData.ClassDeviceObject, InputDataStart, InputDataEnd, InputDataConsumed);
}

#pragma warning(pop)