#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "Core"
using namespace std;

#define CLASSNAME "MXCalibur"
#define MAX_PTS 20000

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct dataset
{
	int index;
	int size;
	POINT target;
};

RECT rect;
COLORREF back = RGB(255, 255, 255);

HANDLE hout;
HWND hwnd;
HDC hdc;
HPEN green;
HPEN red;
HPEN white;
HHOOK hook;

int width;
int height;

POINT datapts[MAX_PTS];
vector<dataset> datasets;
int index = 0;
int state = 0;
dataset train;

struct GRID
{
	int x;
	int y;
	int tx;
	int ty;
};

GRID* gridpts;
int gptsize;

int curr;
int targ;

void Grid(int dimx, int dimy)
{
	double xi = (double)width / (dimx + 1);
	double yi = (double)height / (dimy + 1);
	double axi = 65536.0 / (dimx + 1);
	double ayi = 65536.0 / (dimy + 1);

	gptsize = dimx * dimy;
	gridpts = new GRID[gptsize];
	double x = -1.0, y, ax = -1.0, ay;
	int n = 0;
	for (int i = 1; i <= dimx; i++)
	{
		x += xi;
		y = -1.0;
		ax += axi;
		ay = -1.0;
		for (int j = 1; j <= dimy; j++)
		{
			y += yi;
			ay += ayi;
			gridpts[n].x = round(x);
			gridpts[n].y = round(y);
			gridpts[n].tx = round(ax);
			gridpts[n].ty = round(ay);
			n++;
		}
	}
}

void DrawTarget(int x, int y, int halfwidth)
{
	MoveToEx(hdc, x - halfwidth, y, NULL);
	LineTo(hdc, x + halfwidth, y);
	MoveToEx(hdc, x, y - halfwidth, NULL);
	LineTo(hdc, x, y + halfwidth);
}

int main()
{
	SetConsoleTitleA("MXCalibur");

	hout = GetStdHandle(STD_OUTPUT_HANDLE);

	srand(GetTickCount());

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = GetModuleHandleA(NULL);
	wcex.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wcex.lpszClassName = CLASSNAME;

	if (!RegisterClassEx(&wcex))
		return 1;

	hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, CLASSNAME, CLASSNAME, WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);

	if (!hwnd)
		return 1;

	SetLayeredWindowAttributes(hwnd, back, 0, LWA_COLORKEY);
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hwnd);

	//Assume top-left corner is (0, 0)
	GetWindowRect(hwnd, &rect);
	width = rect.right;
	height = rect.bottom;

	hdc = GetDC(hwnd);
	green = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
	red = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	white = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
	FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

	RAWINPUTDEVICE RID;
	RID.usUsagePage = 1;
	RID.usUsage = 2;
	RID.dwFlags = RIDEV_INPUTSINK;
	RID.hwndTarget = hwnd;
	RegisterRawInputDevices(&RID, 1, sizeof(RAWINPUTDEVICE));

	hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);

	Grid(46, 26);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); //Translates VK to WM_CHAR 
		DispatchMessage(&msg); //Sends message to Window Proc
	}

	UnhookWindowsHookEx(hook);

	DeleteObject(green);
	DeleteObject(red);
	DeleteObject(white);
	ReleaseDC(hwnd, hdc);

	UnregisterClass(CLASSNAME, GetModuleHandle(NULL));

	if (datasets.size() > 0)
	{
		ofstream file("trainingdata.txt");

		int count = datasets.size();
		file << count << endl;
		for (int i = 0; i < count; i++)
			file << datasets[i].size << endl;
		for (int i = 0; i < count; i++)
			file << datasets[i].target.x << ' ' << datasets[i].target.y << endl;
		for (int i = 0; i < index; i++)
			file << datapts[i].x << ' ' << datapts[i].y << endl;
		file.close();
	}

	return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == 0 && wParam != WM_MOUSEMOVE)
		//if (nCode == 0)
		return 1;
	return CallNextHookEx(0, nCode, wParam, lParam);
}

void Next()
{
	SelectObject(hdc, white);
	DrawTarget(gridpts[curr].x, gridpts[curr].y, 15);

	curr = targ;
	int mx = gridpts[curr].tx;
	int my = gridpts[curr].ty;
	do
	{
		targ = (int)(((double)rand() / (RAND_MAX + 1)) * gptsize);
		double tx = gridpts[targ].tx - mx;
		double ty = gridpts[targ].ty - my;
		if (tx * tx + ty * ty > 773094113.28)
			targ = curr;
	} while (curr == targ);

	SelectObject(hdc, green);
	DrawTarget(gridpts[curr].x, gridpts[curr].y, 15);

	SelectObject(hdc, red);
	DrawTarget(gridpts[targ].x, gridpts[targ].y, 15);

	train.index = index;
	train.size = 0;
	train.target.x = gridpts[targ].tx - gridpts[curr].tx;
	train.target.y = gridpts[targ].ty - gridpts[curr].ty;

	SetConsoleCursorPosition(hout, COORD{ 5, 1 });
	cout << datasets.size() << flush;
	SetConsoleCursorPosition(hout, COORD{ 15, 2 });
	cout << train.target.x << ' ' << train.target.y << "                  " << flush;
	SetConsoleCursorPosition(hout, COORD{ 12, 3 });
	cout << "0                  " << endl;
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
			if (raw.data.mouse.usButtonFlags == RI_MOUSE_MIDDLE_BUTTON_UP)
			{
				PostQuitMessage(0);
				break;
			}

			if (state == 0 && raw.data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP)
			{
				cout << "Total Datapoints: 0\nSet: \nTarget Offset: \nDatapoints: " << endl;
				targ = (int)(((double)rand() / (RAND_MAX + 1)) * gptsize);
				Next();
				state = 2;
			}
			else if (state == 1)
			{
				int dx = raw.data.mouse.lLastX;
				int dy = raw.data.mouse.lLastY;
				if (raw.data.mouse.usFlags ^ MOUSE_MOVE_ABSOLUTE && (dx != 0 || dy != 0))
				{
					if (index >= MAX_PTS)
					{
						PostQuitMessage(0);
						break;
					}
					datapts[index].x = dx;
					datapts[index].y = dy;
					train.size++;
					index++;

					SetConsoleCursorPosition(hout, COORD{ 18, 0 });
					cout << index << "                  " << flush;
					SetConsoleCursorPosition(hout, COORD{ 12, 3 });
					cout << train.size << "                  " << endl;
				}
				if (raw.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
				{
					train.size = 0;
					index = train.index;
					state = 2;

					SetConsoleCursorPosition(hout, COORD{ 18, 0 });
					cout << index << "                  " << flush;
					SetConsoleCursorPosition(hout, COORD{ 12, 3 });
					cout << "0                      " << endl;
				}
				else if (raw.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
				{
					if (train.size > 0)
					{
						datasets.push_back(train);
						Next();
						state = 2;
					}
				}
			}
			else if (state == 2 && (raw.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN))
			{
				SetCursorPos(gridpts[curr].x, gridpts[curr].y);
				state = 1;
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}