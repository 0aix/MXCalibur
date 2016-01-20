#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
using namespace std;

#define WS_DEFAULT ((WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME))
#define CLASSNAME "MXCalibur"

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

float x = 0.0f;
float y = 0.0f;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASSNAME;
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
		return 1;

	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRect(&rect, WS_DEFAULT, false);

	HWND hwnd = CreateWindow(CLASSNAME, CLASSNAME, WS_DEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return 1;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	RAWINPUTDEVICE RID;
	RID.usUsagePage = 1;
	RID.usUsage = 2;
	RID.dwFlags = RIDEV_INPUTSINK;
	RID.hwndTarget = hwnd;
	bool res = RegisterRawInputDevices(&RID, 1, sizeof(RAWINPUTDEVICE));

	SetCursorPos(0, 0);

	HHOOK hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); //Translates VK to WM_CHAR 
		DispatchMessageA(&msg); //Sends message to Window Proc
	}

	UnhookWindowsHookEx(hook);

	UnregisterClass(CLASSNAME, hInstance);

	return (int)msg.wParam;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_MOUSEMOVE)
	{
		MSLLHOOKSTRUCT* msll = (MSLLHOOKSTRUCT*)lParam;
		if (msll->dwExtraInfo != 0xABABABAB)
			return 1;
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
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
		if (raw.header.dwType == RIM_TYPEMOUSE && raw.data.mouse.usFlags ^ MOUSE_MOVE_ABSOLUTE)
		{
			const float scale = 20.0f;
			x += raw.data.mouse.lLastX * scale;
			y += raw.data.mouse.lLastY * scale * 1366.0f / 768.0f;
			INPUT input = {};
			input.mi.dx = max(0, min((LONG)x, 65536));
			input.mi.dy = max(0, min((LONG)y, 65536));
			input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
			input.mi.dwExtraInfo = 0xABABABAB;
			SendInput(1, &input, sizeof(INPUT));
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}