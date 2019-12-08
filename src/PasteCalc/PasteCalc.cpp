// PasteCalc.cpp: Файл представлен исключительно в ознакомительных
// целях и не является самостоятельной единицей

#include "stdafx.h"
#include "PasteCalc.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT1));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
#ifdef MENU_ENABLED // Выключает меню без удаления из ресурса
#define HEIGHT 200
#else 
#define HEIGHT 190
#endif
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   HWND hWnd = CreateWindow(szWindowClass, szTitle, 
	~WS_THICKFRAME & WS_OVERLAPPEDWINDOW, // Убрать рамку изменения размера
	CW_USEDEFAULT, 0, 390, HEIGHT, 
	nullptr, nullptr, 
	hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//
HWND hEditBox; // Поле вставки
HWND hLabelResult; // Поле результата
HFONT hFont;
HFONT hFontRes;
// Хендлы дочернего процесса
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

void WriteToPipe(const std::wstring &);
std::wstring ReadFromPipe();


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#define EDIT_WIDTH 300
#define RES_WIDTH 60
#define RES_HEIGHT 30
    switch (message)
    {
	case WM_CREATE:
		// Шрифт в поле вставки
		hFont = CreateFont(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
		// Шрифт в поле результата
		hFontRes = CreateFont(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));

		hEditBox = CreateWindow(TEXT("EDIT"), 0, 
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_AUTOVSCROLL | ES_WANTRETURN,
			5, 10, EDIT_WIDTH, 140, 
			hWnd, 
			(HMENU)IDC_EDIT1, 
			hInst, nullptr);
		if (hEditBox == nullptr)
			return -1;

		hLabelResult = CreateWindow(TEXT("STATIC"), TEXT(" 0"),
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_EX_CLIENTEDGE,
			305, 110, RES_WIDTH, RES_HEIGHT,
			hWnd,
			(HMENU)IDC_STATIC1,
			hInst, nullptr);
		
		if (hLabelResult == nullptr)
			return -1;
		// Установить нормальный шрифт
		SendMessage(hEditBox, WM_SETFONT, WPARAM(hFont), TRUE);
		SendMessage(hLabelResult, WM_SETFONT, WPARAM(hFontRes), FALSE);
		
		
		return 0;
		
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			int eId = HIWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
			case IDC_EDIT1:
			{
				
				if (eId == EN_CHANGE) 
				{
					// Вызов функции парсинга
					int len = GetWindowTextLength(hEditBox);
										
					std::wstring arr;

					arr.resize(len + 1);

					GetWindowText(hEditBox, &arr[0], len + 1);
					
					SECURITY_ATTRIBUTES saAttr;

					saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
					saAttr.bInheritHandle = TRUE;
					saAttr.lpSecurityDescriptor = NULL;

					if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
						return -1;

					if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
						return -1;

					if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
						return -1;

					if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
						return -1;


					TCHAR szCmdline[] = TEXT("prettycalc.exe"); // Путь к консольной программе

					PROCESS_INFORMATION piProcInfo;
					STARTUPINFO siStartInfo;
					BOOL bSuccess = FALSE;

					ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

					ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

					siStartInfo.cb = sizeof(STARTUPINFO);
					siStartInfo.hStdError = g_hChildStd_OUT_Wr;
					siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
					siStartInfo.hStdInput = g_hChildStd_IN_Rd;
					siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

					bSuccess = CreateProcess(NULL,
						szCmdline,
						NULL,
						NULL,
						TRUE,
						CREATE_NO_WINDOW,
						NULL,
						NULL,
						&siStartInfo,
						&piProcInfo);

					bool error = false;

					std::wstring m;

					if (!bSuccess)
						error = true;
					else
					{
						WriteToPipe(arr);
						WaitForSingleObject(piProcInfo.hProcess, INFINITE);
						m = ReadFromPipe();						
					}
					CloseHandle(piProcInfo.hProcess);
					CloseHandle(piProcInfo.hThread);
					
					std::wstringstream sm(m);
					std::wstring temp;

					// Обработаем вывод команды из консоли
					sm >> temp;
					int res = 0;
					if (temp == TEXT("Pretty"))
					{
						sm >> temp; // Calc
						sm >> temp; // Result:
						if (temp != TEXT("Result:")) 
							error = true;
						else
							sm >> res;
					}
					else 
					{
						error = true;
					}
										
					bool over = false;
					
					if (res > 10000) 
					{ 
						error = over = true; 
					}

					std::wstringstream r;
					r << " " << res;
					
					if (!error) 
						SendDlgItemMessage(hWnd, IDC_STATIC1, WM_SETTEXT, 0, (LPARAM)r.str().c_str());
					else 
						SendDlgItemMessage(hWnd, IDC_STATIC1, WM_SETTEXT, 0, (over? LPARAM(TEXT(" !###")): LPARAM(TEXT(" ...")))); 
					
					return 0;
				}					
				
			}
			break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
		
	case WM_SIZE:
	{
		int fwSizeType = (int)wParam;
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);
		
		// Реакция на максимизацию окна и восстановление исходного размера
		if (fwSizeType == SIZE_MAXIMIZED || fwSizeType==SIZE_RESTORED /*|| fwSizeType==SIZE_MAXSHOW*/)
		{
			RECT editRect = { 0 };
			SendDlgItemMessage(hWnd, IDC_EDIT1, EM_GETRECT, 0, (LPARAM) &editRect);
			// Изменение дочерних элементов
			SetWindowPos(hEditBox, nullptr, 
				editRect.left, 10, 
				EDIT_WIDTH - 5, nHeight - 10, 
				SWP_NOZORDER | SWP_SHOWWINDOW);
			
			RECT resRect = { 0 };
			GetWindowRect(hLabelResult, &resRect);
			resRect.left = EDIT_WIDTH + 5;
			resRect.right = resRect.left + RES_WIDTH;
			resRect.top = nHeight - 10 - RES_HEIGHT;
			resRect.bottom = resRect.top + RES_HEIGHT;
			SetWindowPos(hLabelResult, nullptr, 
				resRect.left, resRect.top, 
				0, 0, 
				SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		
	}break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		// Освободим ресурсы
		DeleteObject(hFont);
		DeleteObject(hFontRes);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе". (Не используется)
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// Преобразование из wchar_t в UTF-8
std::string ws2s(const std::wstring& str)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
// Преобразование из UTF-8 в wchar_t
std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void WriteToPipe(const std::wstring &str)
{
	DWORD dwWritten;
	CHAR chBuf[4096] = { 0 };
	
	std::string s = ws2s(str);
	
	WriteFile(g_hChildStd_IN_Wr, s.c_str(), s.size(), &dwWritten, NULL);
	
	CloseHandle(g_hChildStd_IN_Wr);
}

std::wstring ReadFromPipe()
{
	DWORD dwRead;
	CHAR chBuf[4096] = { 0 };
	
	ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
	
	return s2ws(chBuf);
}
