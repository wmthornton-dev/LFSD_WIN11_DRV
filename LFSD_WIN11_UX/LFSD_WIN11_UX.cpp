// LFSD_WIN11_UX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "LFSD_WIN11_UX.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Unknown(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Contribute(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Help(HWND, UINT, WPARAM, LPARAM);
HWND hStatus;

//void EnableDarkMode(HWND hWnd)
//{
//    BOOL darkMode = TRUE;
//    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
//}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LFSDWIN11UX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LFSDWIN11UX));

    MSG msg;

    // Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LFSDWIN11UX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LFSDWIN11UX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, 600, 450, nullptr, nullptr, hInstance, nullptr);

   // Create the status bar
   hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBT_NOBORDERS, L"Ready", hWnd, IDC_STATUSBAR);

   if (!hWnd)
   {
      return FALSE;
   }

   // Get the screen dimensions
   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   // Get the window dimensions
   RECT windowRect;
   GetWindowRect(hWnd, &windowRect);
   int windowWidth = windowRect.right - windowRect.left;
   int windowHeight = windowRect.bottom - windowRect.top;

   // Calculate the position to center the window
   int xPos = (screenWidth - windowWidth) / 2;
   int yPos = (screenHeight - windowHeight) / 2;

   // Set the window position
   SetWindowPos(hWnd, HWND_TOP, xPos, yPos, 0, 0, SWP_NOSIZE);

   //EnableDarkMode(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT_PTR timerId;

    switch (message)
    {
    case WM_CREATE:
        // Set a timer to update the status bar every second
        timerId = SetTimer(hWnd, 1, 1000, NULL);
        break;

    case WM_TIMER:
        if (wParam == 1)
        {
            // Get the current date and time
            SYSTEMTIME st;
            GetLocalTime(&st);
            wchar_t dateTime[100];
            swprintf_s(dateTime, 100, L"%02d/%02d/%04d %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);

            // Update the status bar
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Ready");
            SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)dateTime);
        }
        break;

    case WM_SIZE:
        // Resize the status bar
        SendMessage(hStatus, WM_SIZE, 0, 0);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_HELP:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_HELPBOX), hWnd, Help);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_CONTRIBUTE:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_CONTRIBUTEBOX), hWnd, Contribute);
            break;
        case IDM_UNKNOWN:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_UNKNOWNBOX), hWnd, Unknown);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        KillTimer(hWnd, timerId);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



// Message handler for About box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //EnableDarkMode(hDlg);

        // Check for the Close button of the dialog box
        HMENU hMenu = GetSystemMenu(hDlg, FALSE);
        if (hMenu != NULL)
        {
            // Gray the Close button of the dialog box if it exists
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }

        // Center the dialog box in the main program window
        HWND hWndParent = GetParent(hDlg);
        RECT rcParent, rcDlg;
        GetWindowRect(hWndParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;

        int x = rcParent.left + (parentWidth - dlgWidth) / 2;
        int y = rcParent.top + (parentHeight - dlgHeight) / 2;

        SetWindowPos(hDlg, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

        return (INT_PTR)TRUE;
    }

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

// Message handler for Unknown error box.
INT_PTR CALLBACK Unknown(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //EnableDarkMode(hDlg);

        // Check for the Close button of the dialog box
        HMENU hMenu = GetSystemMenu(hDlg, FALSE);
        if (hMenu != NULL)
        {
            // Gray the Close button of the dialog box if it exists
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }

        // Center the dialog box in the main program window
        HWND hWndParent = GetParent(hDlg);
        RECT rcParent, rcDlg;
        GetWindowRect(hWndParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;

        int x = rcParent.left + (parentWidth - dlgWidth) / 2;
        int y = rcParent.top + (parentHeight - dlgHeight) / 2;

        SetWindowPos(hDlg, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

        return (INT_PTR)TRUE;
    }

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

// Message handler for Contribute dialog box.
INT_PTR CALLBACK Contribute(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //EnableDarkMode(hDlg);

        // Check for the Close button of the dialog box
        HMENU hMenu = GetSystemMenu(hDlg, FALSE);
        if (hMenu != NULL)
        {
            // Gray the Close button of the dialog box if it exists
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }

        // Center the dialog box in the main program window
        HWND hWndParent = GetParent(hDlg);
        RECT rcParent, rcDlg;
        GetWindowRect(hWndParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;

        int x = rcParent.left + (parentWidth - dlgWidth) / 2;
        int y = rcParent.top + (parentHeight - dlgHeight) / 2;

        SetWindowPos(hDlg, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

        return (INT_PTR)TRUE;
    }

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

// Message handler for Help dialog box.
INT_PTR CALLBACK Help(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //EnableDarkMode(hDlg);

        // Check for the Close button of the dialog box
        HMENU hMenu = GetSystemMenu(hDlg, FALSE);
        if (hMenu != NULL)
        {
            // Gray the Close button of the dialog box if it exists
            DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }

        // Center the dialog box in the main program window
        HWND hWndParent = GetParent(hDlg);
        RECT rcParent, rcDlg;
        GetWindowRect(hWndParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;
        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;

        int x = rcParent.left + (parentWidth - dlgWidth) / 2;
        int y = rcParent.top + (parentHeight - dlgHeight) / 2;

        SetWindowPos(hDlg, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

        return (INT_PTR)TRUE;
    }

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