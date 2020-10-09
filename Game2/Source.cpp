#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <io.h>
#define WM_KEYUP        0x0101
#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001

const TCHAR* szClsName = _T("HelloWindowClass123456789");
//----------------------------------------------------------------------------------------
struct rgb {
	double r, g, b;
};

struct Config {
	int		n			= 3;
	int		width		= 320;
	int		height		= 240;
	rgb		color_line	= { 255,	0,	0 };
	rgb		color_field = { 0,		0,	255 };

}cfg;

int** array;
std::string scfg = "config.cfg";
//----------------------------------------------------------------------------------------
void rgbChanger2(rgb& color, double delta)
{
	if(delta > 0)
	{
		if (color.r >= 255 && color.g < 255 && color.b <= 0)
			color.g += delta;
		else if (color.r > 0 && color.g >= 255 && color.b <= 0)
			color.r -= delta;
		else if (color.r <= 0 && color.g >= 255 && color.b < 255)
			color.b += delta;
		else if (color.r <= 0 && color.g > 0 && color.b >= 255)
			color.g -= delta;
		else if (color.r < 255 && color.g <= 0 && color.b >= 255)
			color.r += delta;
		else if (color.r >= 255 && color.g <= 0 && color.b > 0)
			color.b -= delta;
	}
	else
	{
		if (color.r >= 255 && color.g > 0 && color.b <= 0)
			color.g += delta;
		else if (color.r < 255 && color.g >= 255 && color.b <= 0)
			color.r -= delta;
		else if (color.r <= 0 && color.g >= 255 && color.b > 0)
			color.b += delta;
		else if (color.r <= 0 && color.g < 255 && color.b >= 255)
			color.g -= delta;
		else if (color.r > 0 && color.g <= 0 && color.b >= 255)
			color.r += delta;
		else if (color.r >= 255 && color.g <= 0 && color.b < 255)
			color.b -= delta;
	}
	if (color.r < 0)
		color.r = 255;
	else if (color.r > 255)
		color.r = 0;

	if (color.b < 0)
		color.r = 255;
	else if (color.r > 255)
		color.r = 0;

	if (color.g < 0)
		color.r = 255;
	else if (color.r > 255)
		color.r = 0;
}

void RunNotepad(void)
{
	STARTUPINFO sInfo;
	PROCESS_INFORMATION pInfo;
	ZeroMemory(&sInfo, sizeof(STARTUPINFO));
	CreateProcess(_T("C:\\Windows\\Notepad.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}

bool FileExists(const char* fname)
{
	return _access(fname, 0) != -1;
}

void saveConfig(Config cfg) 
{
	std::ofstream fout(scfg);
	fout << "n = "				<< cfg.n				<< "\n";
	fout << "widht = "			<< cfg.width			<< "\n";
	fout << "height = "			<< cfg.height			<< "\n";
	fout << "color_line = "		<< cfg.color_line.r		<< " "	<< cfg.color_line.g		<< " "	<< cfg.color_line.b		<< "\n";
	fout << "color_field = "	<< cfg.color_field.r	<< " "	<< cfg.color_field.g	<< " "	<< cfg.color_field.b	<< "\n";
	fout.close();
}

void loadConfig(Config& cfg) 
{
	std::ifstream fin(scfg);
	if(FileExists(scfg.c_str()))
	{
		std::string line;
		while (std::getline(fin, line)) {
			std::istringstream sin(line.substr(line.find("=") + 1));
			if (line.find("n ") != -1)
				sin >> cfg.n;
			else if (line.find("widht") != -1)
				sin >> cfg.width;
			else if (line.find("height") != -1)
				sin >> cfg.height;
			else if (line.find("color_line") != -1) 
			{
				sin >> cfg.color_line.r;
				sin >> cfg.color_line.g;
				sin >> cfg.color_line.b;
			}
			else if (line.find("color_field") != -1)
			{
				sin >> cfg.color_field.r;
				sin >> cfg.color_field.g;
				sin >> cfg.color_field.b;
			}
		}
		fin.close();
	}
	else {
		saveConfig(cfg);
	}

}
//----------------------------------------------------------------------------------------
RECT		rect;
LPRECT		lprect	= new tagRECT;
HPEN		hPen;
PAINTSTRUCT ps;
HDC			hdc;

LRESULT CALLBACK WndProc(
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	
	GetClientRect(hWnd, lprect);
	InvalidateRect(hWnd, 0, 0);
	HBRUSH ElipseBr;
	int a = (lprect->right - lprect->left) / cfg.n;
	int b = (lprect->bottom - lprect->top) / cfg.n;
	int x, y;
	HBRUSH col = NULL;
	switch (message)
	{

	case WM_RBUTTONUP:
		x = GET_X_LPARAM(lParam); y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < cfg.n; i++)
		{
			for (int j = 0; j < cfg.n; j++)
			{
				int xl = a	* i;
				int xr = xl + a;
				int yt = b	* j;
				int yb = yt + b;

				if ((xl <= x && yt <= y) && (xr > x && yb > y))
				{
					if (array[i][j] == 2)
					{
						array[i][j] = 0;
					}
					else
						array[i][j] = 2;
					break;
				}
			}
		}
		InvalidateRect(hWnd, lprect, 1);
		return 0;

	case WM_LBUTTONUP:

		x = GET_X_LPARAM(lParam); y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < cfg.n; i++)
		{
			for (int j = 0; j < cfg.n; j++)
			{
				int xl = a	* i;
				int xr = xl + a;
				int yt = b	* j;
				int yb = yt + b;

				if ((xl <= x && yt <= y) && (xr > x && yb > y))
				{
					if (array[i][j] == 1)
					{
						array[i][j] = 0;
					}
					else
						array[i][j] = 1;
					break;
				}
			}
		}
		InvalidateRect(hWnd, lprect, 1);
		return 0;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		for (int i = 0; i < cfg.n; i++)
		{
			for (int j = 0; j < cfg.n; j++)
			{

				if (array[i][j] == 1)
				{
					HDC hdc = GetDC(hWnd);
					HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
					SelectObject(hdc, hBrush);
					int xl = a	* i;
					int xr = xl + a;
					int yt = b	* j;
					int yb = yt	+ b;
					Ellipse(hdc,	xl + a / 20 + 3,	yt + a / 20, xr - a / 20 - 3,	yb - a / 20);
					DeleteObject(hBrush);
					DeleteObject(hdc);
				}
				if (array[i][j] == 2)
				{
					HDC hdc = GetDC(hWnd);
					hPen = CreatePen(PS_SOLID,	4,	RGB(255, 255, 255));
					SelectObject(hdc, hPen);
					int xl = a	* i;
					int xr = xl + a;
					int yt = b	* j;
					int yb = yt + b;

					MoveToEx(hdc,	xl + a - a / 10,	yb - a / 10,	0);
					LineTo	(hdc,	xr - a + a / 10,	yt + a / 10);

					MoveToEx(hdc,	xl + a / 10,		yb - a / 10,	0);
					LineTo	(hdc,	xr - a / 10,		yt + a / 10);

					DeleteObject(hPen);
					DeleteObject(hdc);
				}

			}
		}
		GetClientRect(hWnd, lprect);
		hPen = CreatePen(PS_SOLID, 5, RGB(cfg.color_line.r, cfg.color_line.g, cfg.color_line.b));
		SelectObject(hdc, hPen);

		for (int i = 0; i <= cfg.n; i++)
		{
			MoveToEx(hdc,	lprect->left + a * i,	lprect->top,	0);
			LineTo	(hdc,	lprect->left + a * i,	lprect->bottom);

		}
		for (int i = 1; i <= cfg.n; i++)
		{

			MoveToEx(hdc,	lprect->left,	lprect->top + b * i,	0);
			LineTo	(hdc,	lprect->right,	lprect->top + b * i);

		}

		DeleteObject(hPen);
		DeleteObject(hdc);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		GetWindowRect(hWnd, lprect);
		cfg.width = lprect->right - lprect->left;
		cfg.height = lprect->bottom - lprect->top;
		saveConfig(cfg);					// сохраняем конфиг

		PostQuitMessage(0);
		return 0;

	case WM_MOUSEWHEEL:

		if(GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			rgbChanger2(cfg.color_line, 5);
		else
			rgbChanger2(cfg.color_line, -5);
		
		InvalidateRect(hWnd, 0, true);
		return 0;

	case WM_KEYDOWN:

		printf("%d\n", wParam);
		if (wParam == VK_RETURN) {				//Enter
			cfg.color_field.r = rand() % 256;
			cfg.color_field.g = rand() % 256;
			cfg.color_field.b = rand() % 256;
			col = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(cfg.color_field.r, cfg.color_field.g, cfg.color_field.b)));
			InvalidateRect(hWnd, 0, true);
			DeleteObject(col);
		}

		if (wParam == VK_ESCAPE)				//Esc
		{
			PostQuitMessage(0);					//Завершаем работу
			return 0;							//
		}

		if (wParam == 0x51)						// ctrl + q
		{
			if (GetKeyState(VK_CONTROL) < 0)	//
				PostQuitMessage(0);				//Завершаем работу
			return 0;
		}

		if (wParam == 0x43)						//shift + c
		{
			if (GetKeyState(VK_SHIFT) < 0)		//
				RunNotepad();					//Запускаем блокнот
			return 0;
		}

		if (wParam == VK_SPACE)					//space для установки дефолтных значений
		{
			cfg.n = 3;
			cfg.width = 320;
			cfg.height = 240;
			cfg.color_line = { 255, 0, 0 };
			cfg.color_field = { 0, 0, 255 };
			col = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(cfg.color_field.r, cfg.color_field.g, cfg.color_field.b)));
			DeleteObject(col);
			GetWindowRect(hWnd, lprect);
			MoveWindow(hWnd,lprect->left, lprect->top, cfg.width, cfg.height, TRUE); //->Move window
			for (int i = 0; i < cfg.n; i++)
				for (int j = 0; j < cfg.n; j++)
					array[i][j] = 0;
			InvalidateRect(hWnd, 0, true);

			return 0;
		}

		return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
	srand(time(NULL));
	loadConfig(cfg);

	if (argc > 1)
	{
		cfg.n = atoi(argv[1]);
	}

	array = new int* [cfg.n];
	for (int i = 0; i < cfg.n; i++)
	{
		array[i] = new int [cfg.n];
	}

	HINSTANCE	hThis	= GetModuleHandle(NULL);
	WNDCLASS	cls		= { 0 };
	cls.hInstance		= hThis;
	cls.lpszClassName	= szClsName;
	cls.lpfnWndProc		= WndProc;

	if (!RegisterClass(&cls))
	{
		printf("%A\n", GetLastError());
	}

	HWND hWnd = CreateWindow(
		szClsName,
		_T("Hello Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		cfg.width,
		cfg.height,
		HWND_DESKTOP,
		NULL,
		hThis,
		NULL
	);
	(HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(cfg.color_field.r, cfg.color_field.g, cfg.color_field.b)));
	ShowWindow(hWnd, SW_SHOW);


	BOOL bOk;
	MSG msg;

	while ((bOk = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bOk < 0)
		{
			printf("%A\n", GetLastError());
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hWnd);
	UnregisterClass(szClsName, hThis);
	return 0;
}