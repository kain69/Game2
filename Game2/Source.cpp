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
#define STACK_SIZE		(64*1024)

const TCHAR* szClsName = _T("HelloWindowClass");

HANDLE procsem;
HANDLE stepevent;
HANDLE stepmutex;
HANDLE gameoverMutex;
HANDLE animevent;
CRITICAL_SECTION gameoverCrit;

RECT		rect;
LPRECT		lprect = new tagRECT;
HPEN		hPen;
PAINTSTRUCT ps;
HDC			hdc;
//----------------------------------------------------------------------------------------
struct rgb {
	double r, g, b;
};

struct Config {
	int		n			= 3;
	int		width		= 320;
	int		height		= 240;
	rgb		color_line	= { 255,	0,	0   };
	rgb		color_field = { 0,		0,	255 };
	rgb     cross		= { 255,  255,  255 };
	rgb     zero		= { 100,  100,  100 };

}cfg;

UINT WM_UPDATE_MESSAGE = RegisterWindowMessage(0);
UINT WM_GET_COUNT = RegisterWindowMessage(L"Please");
int** array;
int countStep = 0;
bool flagAnim = true;
HBRUSH col = NULL;
HWND hWnd;
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

DWORD WINAPI AnimSpin(LPVOID p) {
	while (true) {
		rgbChanger2(cfg.color_line, 5);
		rgbChanger2(cfg.color_field, 5);
		rgbChanger2(cfg.cross, 17);
		rgbChanger2(cfg.zero, 17);

		col = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(cfg.color_field.r, cfg.color_field.g, cfg.color_field.b)));
		DeleteObject(col);

		InvalidateRect(hWnd, lprect, 1);
		Sleep(50);
	}
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
	fout << "cross = "	<< cfg.cross.r	<< " "	<< cfg.cross.g	<< " "	<< cfg.cross.b	<< "\n";
	fout << "zero = "	<< cfg.zero.r	<< " "	<< cfg.zero.g	<< " "	<< cfg.zero.b	<< "\n";
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
			else if (line.find("cross") != -1)
			{
				sin >> cfg.cross.r;
				sin >> cfg.cross.g;
				sin >> cfg.cross.b;
			}
			else if (line.find("zero") != -1)
			{
				sin >> cfg.zero.r;
				sin >> cfg.zero.g;
				sin >> cfg.zero.b;
			}
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
		if (wcscmp(szTextWin, TEXT("HelloWindow")) == 0 && hwnd != hWnd)
		{
			/*std::cout << "hwnd: " << hwnd << "\n" << "name: ";
			_tprintf(szTextWin);
			std::cout << "\n";*/
			if (lParam == 0)
			{
				SendMessage(hwnd, WM_UPDATE_MESSAGE, countStep, 0);
				std::cout << "Закинул countStep и обнову\n";
			}	
			if (lParam == 1)
			{
				SendMessage(hwnd, WM_GET_COUNT, 0, 0);
				std::cout << "Запросил countStep\n";
			}
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
	for (int column = 0; column < cfg.n; column++) {
		cols = true;
		rows = true;
		for (int row = 0; row < cfg.n; row++) {
			cols &= (array[column][row] == symb);
			rows &= (array[row][column] == symb);
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
		if (WaitForSingleObject(gameoverMutex, 0) == WAIT_TIMEOUT) {
			PostQuitMessage(0);
			return;
		}

		MessageBox(NULL, _T("Game Over! Win Нолик"), _T("TickTackToe"),
			MB_OK | MB_SETFOREGROUND);

		PostQuitMessage(0);
	}
	if (checkDiagonal(array, 2) || checkLane(array, 2))
	{
		if (WaitForSingleObject(gameoverMutex, 0) == WAIT_TIMEOUT) {
			PostQuitMessage(0);
			return;
		}

		MessageBox(NULL, _T("Game Over! Win Крестик"), _T("TickTackToe"),
			MB_OK | MB_SETFOREGROUND);
		
		PostQuitMessage(0);
	}
	if (countStep >= 9)
	{
		if (WaitForSingleObject(gameoverMutex, 0) == WAIT_TIMEOUT) {
			PostQuitMessage(0);
			return;
		}

		MessageBox(NULL, _T("Game Over! Ничья"), _T("TickTackToe"),
			MB_OK | MB_SETFOREGROUND);

		PostQuitMessage(0);
	}
}
//----------------------------------------------------------------------------------------

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
	double a = (lprect->right - lprect->left) / (double)cfg.n;
	double b = (lprect->bottom - lprect->top) / (double)cfg.n;
	int x, y;
	

	if (message == WM_UPDATE_MESSAGE) {
		countStep = wParam;

		if (WaitForSingleObject(stepevent, 0) == WAIT_TIMEOUT)
			SetEvent(stepevent);
		else
			ResetEvent(stepevent);

		InvalidateRect(hWnd, lprect, 1);
		checkEnd(array);
	}
	if (message == WM_GET_COUNT)
		EnumWindows(&EnumWindowsProc, 0);
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
			if(countStep % 2 == 0){
				if(WaitForSingleObject(stepevent, 0) == WAIT_TIMEOUT)
					return 0;
			}
			else
				if (WaitForSingleObject(stepevent, 0) == WAIT_TIMEOUT) {
					return 0;
				}

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
			InvalidateRect(hWnd, lprect, 1);

			if (WaitForSingleObject(stepevent, 0) == WAIT_TIMEOUT)
				SetEvent(stepevent);
			else
				ResetEvent(stepevent);

			checkEnd(array);
			EnumWindows(&EnumWindowsProc, 0);
			
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
						HBRUSH hBrush = CreateSolidBrush(RGB(cfg.zero.r, cfg.zero.g, cfg.zero.b));
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
						hPen = CreatePen(PS_SOLID, 4, RGB(cfg.cross.r, cfg.cross.g, cfg.cross.b));
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

		/*case WM_MOUSEWHEEL:

			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				rgbChanger2(cfg.color_line, 5);
			else
				rgbChanger2(cfg.color_line, -5);

			InvalidateRect(hWnd, 0, true);
			return 0;*/

		case WM_KEYDOWN:

			printf("%d\n", wParam);
			if (wParam == VK_RETURN) {				//Enter
				if(WaitForSingleObject(animevent, 0) == WAIT_TIMEOUT){
					SuspendThread(AnimSpin);
					SetEvent(animevent);
					std::cout << "ПАУЗА ЕПТА! \n";
					return 0;
				}
				std::cout << "СТАРТ ЕПТА! \n";
				ResumeThread(AnimSpin);
				ResetEvent(animevent);
			}
			//	cfg.color_field.r = rand() % 256;
			//	cfg.color_field.g = rand() % 256;
			//	cfg.color_field.b = rand() % 256;
			//	col = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(cfg.color_field.r, cfg.color_field.g, cfg.color_field.b)));
			//	InvalidateRect(hWnd, 0, true);
			//	DeleteObject(col);
			//}

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
				cfg.cross = {0, 255, 0};
				cfg.zero = {0, 255, 255};
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

	//////////////////////////////////////////////////////////////////////////////////////////////

	procsem = CreateSemaphore(NULL, 2, 2, L"Global\MySemaphore");

	if (procsem == NULL)
	{
		procsem == OpenSemaphore(
			SEMAPHORE_ALL_ACCESS,
			FALSE,
			L"Global\MySemaphore");
	}

	if (WaitForSingleObject(procsem, 0) == WAIT_TIMEOUT) {
		CloseHandle(procsem);
		PostQuitMessage(0);
	}

	gameoverMutex = CreateMutexW(NULL, NULL, L"Global\MyMutex");

	if(gameoverMutex == NULL)
	{
		gameoverMutex == OpenMutex(
			MUTEX_ALL_ACCESS,
			FALSE,
			L"Global\MyMutex");
	}

	InitializeCriticalSection(&gameoverCrit);

	stepevent = CreateEvent(
		NULL,
		TRUE,
		TRUE,
		NULL);

	animevent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"Global\MyEvent");

	if (animevent == NULL)
	{
		animevent == OpenEvent(
			EVENT_ALL_ACCESS,
			FALSE,
			L"Global\MyEvent");
	}

	CreateThread(NULL, STACK_SIZE, AnimSpin, NULL, 0, NULL);

	/*stepmutex = CreateMutexW(
		NULL,
		NULL,
		L"Global\MyStepMutex");

	if (stepmutex == NULL)
	{
		stepmutex == OpenMutex(
			MUTEX_ALL_ACCESS,
			FALSE,
			L"Global\MyStepMutex");
	}*/

	//////////////////////////////////////////////////////////////////////////////////////////////

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

	hWnd = CreateWindow(
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

	

	array = new int* [cfg.n];
	for (int i = 0; i < cfg.n; i++)
	{
		array[i] = pBuf + cfg.n * i;
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		printf("Hello i`m creator\n");
		//CopyMemory((PVOID)pBuf, array, sizeof(array) * cfg.n * cfg.n);
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Hello i`m listener\n");
		EnumWindows(&EnumWindowsProc, 1);
	}

	//std::cout << hWnd << "\n";
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


	ReleaseSemaphore(procsem, 1, NULL);

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	DestroyWindow(hWnd);
	UnregisterClass(szClsName, hThis);
	return 0;
}