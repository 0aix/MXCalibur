#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <fstream>
#include "Eigen"
using namespace std;

#define CLASSNAME "MXArtemis"

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

Eigen::MatrixXd W1(2, 4);
Eigen::MatrixXd W2(4, 4);
Eigen::MatrixXd W3(4, 2);
Eigen::MatrixXd b1(1, 4);
Eigen::MatrixXd b2(1, 4);
Eigen::MatrixXd b3(1, 2);

void LoadWeights(Eigen::MatrixXd& m, ifstream& weights)
{
	double buff[64];
	int rows = m.rows();
	int cols = m.cols();
	int count = rows * cols;
	for (int i = 0; i < count; i++)
		weights >> buff[i];
	int n = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			m(i, j) = buff[n];
			n++;
		}
	}
}

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
	SetConsoleTitleA("MXArtemis");

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

	//char buff[256];
	//cin >> buff;

	ifstream weights("weights.txt");
	LoadWeights(W1, weights);
	LoadWeights(W2, weights);
	LoadWeights(W3, weights);
	LoadWeights(b1, weights);
	LoadWeights(b2, weights);
	LoadWeights(b3, weights);
	weights.close();

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
			if (raw.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
				overwatch = !overwatch;
			if (dx != 0 || dy != 0)
			{
				//Eigen::MatrixXd X1(1, 2);
				//Eigen::MatrixXd X2(1, 4);
				//Eigen::MatrixXd X3(1, 4);
				Eigen::MatrixXd X4(1, 2);

				//X1 << dx, dy;
				//X2 = X1 * W1 + b1;
				//X2 = X2.cwiseSign().cwiseProduct(X2.cwiseAbs().array().pow(1.42).matrix());
				//X3 = X2 * W2 + b2;
				//X4 = X3 * W3 + b3;

				double v = sqrt((double)(dx * dx + dy * dy));
				v = 2.159667192317668 + v * (0.910082601888066 + v * (-0.013081352897007546 + 0.00006161701350819924 * v));
				X4(0, 0) = dx * v;
				X4(0, 1) = dy * v * 1366.0 / 768.0;

				INPUT input = {};
				if (!overwatch)
				{
					POINT p;
					GetCursorPos(&p);
					if (abs(p.x - posx) > 5 || abs(p.y - posy) > 5)
					{
						GetAbsolutePosition(ax, ay);
						//ax = p.x / 1366.0 * 65535.0;
						//ay = p.y / 768.0 * 65535.0;
					}

					ax += X4(0, 0);
					ay += X4(0, 1);
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
					const double x_sensitivity = 0.352927;
					const double y_sensitivity = 0.214394;
					ax = x_sensitivity * X4(0, 0) - dx + lx;
					ay = y_sensitivity * X4(0, 1) - dy + ly;
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