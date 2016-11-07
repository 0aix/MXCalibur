#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
using namespace std;

#define CLASSNAME "MXGemini"

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

double ax;
double ay;

void GetAbsolutePosition(double& x, double& y)
{
	POINT p;
	MOUSEMOVEPOINT pt = {};
	MOUSEMOVEPOINT ppt;

	while (true)
	{
		GetCursorPos(&p);
		pt.x = p.x;
		pt.y = p.y;
		if (GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), &pt, &ppt, 1, GMMP_USE_HIGH_RESOLUTION_POINTS) == 1)
			break;
	}
	x = ppt.x;
	y = ppt.y;
}

int main()
{
	SetConsoleTitleA("MXGemini");

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = GetModuleHandleA(NULL);
	wcex.lpszClassName = CLASSNAME;

	if (!RegisterClassEx(&wcex))
		return 1;

	HWND hwnd = CreateWindow(CLASSNAME, CLASSNAME, 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);

	if (!hwnd)
		return 1;

	RAWINPUTDEVICE RID;
	RID.usUsagePage = 1;
	RID.usUsage = 2;
	RID.dwFlags = RIDEV_INPUTSINK;
	RID.hwndTarget = hwnd;
	RegisterRawInputDevices(&RID, 1, sizeof(RAWINPUTDEVICE));

	HHOOK hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);

	GetAbsolutePosition(ax, ay);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); //Translates VK to WM_CHAR 
		DispatchMessage(&msg); //Sends message to Window Proc
	}

	UnhookWindowsHookEx(hook);

	UnregisterClass(CLASSNAME, GetModuleHandle(NULL));

	return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == 0 && wParam == WM_MOUSEMOVE && ((MSLLHOOKSTRUCT*)lParam)->dwExtraInfo != 0x5FC1AEDB)
		return 1;
	return CallNextHookEx(0, nCode, wParam, lParam);
}

int posx;
int posy;

bool overwatch = false;
double lx = 0.0, ly = 0.0;

struct InfPoint
{
	double upper;
	double x;
	double intercept;
	double slope;
};

InfPoint infpts[] =
{
	{ 0.4300079345703125, 0.0, 0.0, 2.488946453284127603704623682623 },
	{ 1.25, 0.4300079345703125, 1.0702667236328125, 3.7443755931446435549600848545749 },
	{ 3.8600006103515625, 1.25, 4.140625, 5.6872592064262287414717420154459 },
	{ DBL_MAX, 3.8600006103515625, 18.984375, 11.753337912940458211225723261969 }
};

double MouseToPointer(double v)
{
	double absv = abs(v) / 3.5;

	for (int i = 0; i < sizeof(infpts); i++)
		if (absv <= infpts[i].upper)
			return (absv - infpts[i].x) * infpts[i].slope + infpts[i].intercept;

	return 0.0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD dwSize;
	RAWINPUT raw;
	switch (msg)
	{
	case WM_INPUT:
		dwSize = sizeof(RAWINPUT);
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, (PUINT)&dwSize, sizeof(RAWINPUTHEADER));
		if (raw.header.dwType == RIM_TYPEMOUSE && raw.data.mouse.usFlags ^ MOUSE_MOVE_ABSOLUTE && raw.data.mouse.ulExtraInformation != 0x5FC1AEDB)
		{
			int dx = raw.data.mouse.lLastX;
			int dy = raw.data.mouse.lLastY;
			if (raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
				overwatch = !overwatch;
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

				INPUT input = {};
				if (!overwatch)
				{
					POINT p;
					GetCursorPos(&p);
					if (abs(p.x - posx) > 5 || abs(p.y - posy) > 5)
						GetAbsolutePosition(ax, ay);

					ax += DX;
					ay += DY;
					ax = max(0.0, min(ax, 65535.0));
					ay = max(0.0, min(ay, 65535.0));

					posx = (int)(ax / 65535.0 * 1366.0);
					posy = (int)(ay / 65535.0 * 768.0);

					input.mi.dx = round(ax);
					input.mi.dy = round(ay);
					input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
					input.mi.dwExtraInfo = 0x5FC1AEDB;
					SendInput(1, &input, sizeof(INPUT));
				}
				else
				{
					const double x_sensitivity = 0.286111;
					const double y_sensitivity = 0.195922;
					ax = x_sensitivity * DX - dx + lx;
					ay = y_sensitivity * DY - dy + ly;
					double rx = round(ax);
					double ry = round(ay);
					lx = ax - rx;
					ly = ay - ry;
					input.mi.dx = rx;
					input.mi.dy = ry;
					input.mi.dwFlags = MOUSEEVENTF_MOVE;
					input.mi.dwExtraInfo = 0x5FC1AEDB;
					SendInput(1, &input, sizeof(INPUT));
				}
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}