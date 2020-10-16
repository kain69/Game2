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

const TCHAR* szClsName = _T("HelloWindowClass");
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

UINT WM_UPDATE_MESSAGE = RegisterWindowMessage(0);
int** array;
int countStep = 0;
std::string scfg = "P:\\Влад\\ВУЗ\\2 курс\\ОС\\Game2\\config.cfg";
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

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	TCHAR szTextWin[255];
	if (GetWindowText(hwnd, (LPWSTR)szTextWin, sizeof(szTextWin)))
	{
		if (wcscmp(szTextWin, TEXT("HelloWindow")) == 0)
		{
			/*std::cout << "hwnd: " << hwnd << "\n" << "name: ";
			_tprintf(szTextWin);
			std::cout << "\n";*/
			SendMessage(hwnd, WM_UPDATE_MESSAGE, countStep, NULL);
		}
	}
	return TRUE;
}

bool checkDiagonal(int** array, int symb)
{
	bool toright = true;
	bool toleft = true;
	for (int i = 0; i < cfg.n; i++) {
		toright &= (array[i][i] == symb);
		toleft &= (array[cfg.n - i - 1][i] == symb);
	}
	if (toright || toleft) return true;

	return false;
}
bool checkLane(int** array, int symb)
{
	bool cols = true;
	bool rows = true;
	for (int col = 0; col < cfg.n; col++) {
		cols = true;
		rows = true;
		for (int row = 0; row < cfg.n; row++) {
			cols &= (array[col][row] == symb);
			rows &= (array[row][col] == symb);
		}

		// Это условие после каждой проверки колонки и столбца
		// позволяет остановить дальнейшее выполнение, без проверки 
		// всех остальных столбцов и строк.
		if (cols || rows) return true;
	}
	return false;

}
void checkEnd(int** array)
{
	if(checkDiagonal(array, 1) || checkLane(array, 1))
	{
		for (int i = 0; i < cfg.n; i++)
			for (int j = 0; j < cfg.n; j++)
				array[i][j] = 0;
		countStep = 0;
		EnumWindows(&EnumWindowsProc, 0);
		std::cout << "Game Over, Выиграл нолик\n";
	}
	if (checkDiagonal(array, 2) || checkLane(array, 2))
	{
		for (int i = 0; i < cfg.n; i++)
			for (int j = 0; j < cfg.n; j++)
				array[i][j] = 0;
		countStep = 0;
		EnumWindows(&EnumWindowsProc, 0);
		std::cout << "Game Over, Выиграл крестик\n";
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

	if (message == WM_UPDATE_MESSAGE) {
		countStep = wParam;
		InvalidateRect(hWnd, lprect, 1);
	}
	else
	{
		switch (message)
		{

		/*case WM_RBUTTONUP:
		{
			x = GET_X_LPARAM(lParam); y = GET_Y_LPARAM(lParam);
			for (int i = 0; i < cfg.n; i++)
			{
				for (int j = 0; j < cfg.n; j++)
				{
					int xl = a * i;
					int xr = xl + a;
					int yt = b * j;
					int yb = yt + b;

					if ((xl <= x && yt <= y) && (xr > x && yb > y))
					{
						if (array[i][j] != 1 && array[i][j] != 2)
						{
							array[i][j] = 2;
						}
						break;
					}
				}
			}
			
			EnumWindows(&EnumWindowsProc, 0);
			InvalidateRect(hWnd, lprect, 1);
			return 0;
		}*/
		case WM_LBUTTONUP:
		{
			x = GET_X_LPARAM(lParam); y = GET_Y_LPARAM(lParam);
			for (int i = 0; i < cfg.n; i++)
			{
				for (int j = 0; j < cfg.n; j++)
				{
					int xl = a * i;
					int xr = xl + a;
					int yt = b * j;
					int yb = yt + b;

					if ((xl <= x && yt <= y) && (xr > x && yb > y))
					{
						if (array[i][j] != 1 && array[i][j] != 2)
						{
							if (countStep % 2 == 0)
								array[i][j] = 2;
							else
								array[i][j] = 1;
							countStep++;
						}
						break;
					}
				}
			}
			EnumWindows(&EnumWindowsProc, 0);
			InvalidateRect(hWnd, lprect, 1);
			checkEnd(array);
			return 0;
		}
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
						int xl = a * i;
						int xr = xl + a;
						int yt = b * j;
						int yb = yt + b;
						Ellipse(hdc, xl + a / 20 + 3, yt + a / 20, xr - a / 20 - 3, yb - a / 20);
						DeleteObject(hBrush);
						DeleteObject(hdc);
					}
					if (array[i][j] == 2)
					{
						HDC hdc = GetDC(hWnd);
						hPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
						SelectObject(hdc, hPen);
						int xl = a * i;
						int xr = xl + a;
						int yt = b * j;
						int yb = yt + b;

						MoveToEx(hdc, xl + a - a / 10, yb - a / 10, 0);
						LineTo(hdc, xr - a + a / 10, yt + a / 10);

						MoveToEx(hdc, xl + a / 10, yb - a / 10, 0);
						LineTo(hdc, xr - a / 10, yt + a / 10);

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
				MoveToEx(hdc, lprect->left + a * i, lprect->top, 0);
				LineTo(hdc, lprect->left + a * i, lprect->bottom);

			}
			for (int i = 1; i <= cfg.n; i++)
			{

				MoveToEx(hdc, lprect->left, lprect->top + b * i, 0);
				LineTo(hdc, lprect->right, lprect->top + b * i);

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

			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
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
				MoveWindow(hWnd, lprect->left, lprect->top, cfg.width, cfg.height, TRUE); //->Move window
				for (int i = 0; i < cfg.n; i++)
					for (int j = 0; j < cfg.n; j++)
						array[i][j] = 0;
				InvalidateRect(hWnd, 0, true);

				return 0;
			}

			return 0;

		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "ru");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	//FreeConsole();

	srand(time(NULL));
	loadConfig(cfg);

	if (argc > 1)
	{
		cfg.n = atoi(argv[1]);
	}

	HINSTANCE	hThis	= GetModuleHandle(NULL);
	WNDCLASS	cls		= { 0 };
	cls.hInstance		= hThis;
	cls.lpszClassName	= szClsName;
	cls.lpfnWndProc		= WndProc;

	if (!RegisterClass(&cls))
	{
		printf("%d\n", GetLastError());
	}

	HWND hWnd = CreateWindow(
		szClsName,
		_T("HelloWindow"),
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

	//////////////////////////////////////////////////////////////////////////////////////////////
	HANDLE	hMapFile;

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,				// без файла на диске
		NULL,								// не наследуется процессами-потомками
		PAGE_READWRITE,
		0,
		sizeof(array) * cfg.n * cfg.n,		// размер максимальный
		_T("TestMapping"));		// имя

	if (hMapFile == NULL)
	{
		return printf("CreateFileMapping error: %d\n", GetLastError());
	}

	int* pBuf = (int*)MapViewOfFile(hMapFile,	// handle to map object
		FILE_MAP_READ | FILE_MAP_WRITE,		// read/write permission
		0,
		0,
		0);

	if (pBuf == NULL)
	{
		printf("Error mapping file data: %d\n", GetLastError());

		CloseHandle(hMapFile);

		return 1;
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		printf("Hello i`m creator\n");
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Hello i`m listener\n");
	}

	array = new int* [cfg.n];
	for (int i = 0; i < cfg.n; i++)
	{
		array[i] = pBuf + cfg.n * i;
	}

	CopyMemory((PVOID)pBuf, array, sizeof(array) * cfg.n * cfg.n);


	std::cout << hWnd << "\n";
	//////////////////////////////////////////////////////////////////////////////////////////////

	BOOL bOk;
	MSG msg;

	while ((bOk = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bOk < 0)
		{
			printf("%d\n", GetLastError());
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	DestroyWindow(hWnd);
	UnregisterClass(szClsName, hThis);
	return 0;
}