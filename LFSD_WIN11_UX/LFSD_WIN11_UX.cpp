// LFSD_WIN11_UX.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "LFSD_WIN11_UX.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

HIMAGELIST hImageList;


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
HWND hListView;
void UpdateStatusBarText(int part, const wchar_t* text);


//void EnableDarkMode(HWND hWnd)
//{
//    BOOL darkMode = TRUE;
//    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
//}

void UpdateStatusBarText(int part, const wchar_t* text)
{
    SendMessage(hStatus, SB_SETTEXT, part, (LPARAM)text);
}

void PopulateDrives()
{
    // Clear the existing items
    ListView_DeleteAllItems(hListView);
    ImageList_RemoveAll(hImageList);

    // Add the drives and partitions to the ListView
    DWORD drives = GetLogicalDrives();
    WCHAR driveName[] = L"A:\\";
    for (int i = 0; i < 26; ++i)
    {
        if (drives & (1 << i))
        {
            driveName[0] = L'A' + i;

            // Get drive type
            UINT driveType = GetDriveType(driveName);
            const wchar_t* driveTypeStr = L"Unknown";
            switch (driveType)
            {
            case DRIVE_REMOVABLE: driveTypeStr = L"Removable"; break;
            case DRIVE_FIXED: driveTypeStr = L"Fixed"; break;
            case DRIVE_REMOTE: driveTypeStr = L"Network"; break;
            case DRIVE_CDROM: driveTypeStr = L"CD-ROM"; break;
            case DRIVE_RAMDISK: driveTypeStr = L"RAM Disk"; break;
            }

            // Get file system type
            WCHAR fileSystemName[MAX_PATH];
            GetVolumeInformation(driveName, NULL, 0, NULL, NULL, NULL, fileSystemName, MAX_PATH);

            // Get total and used size
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            GetDiskFreeSpaceEx(driveName, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes);
            ULONGLONG usedBytes = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;

            // Get the drive icon
            SHFILEINFO shFileInfo;
            SHGetFileInfo(driveName, 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
            int iconIndex = ImageList_AddIcon(hImageList, shFileInfo.hIcon);
            DestroyIcon(shFileInfo.hIcon);

            // Insert the drive information into the ListView
            LVITEM lvItem = { 0 };
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iItem = ListView_GetItemCount(hListView);
            lvItem.iSubItem = 0;
            lvItem.pszText = driveName;
            lvItem.iImage = iconIndex;
            ListView_InsertItem(hListView, &lvItem);

            //ListView_SetItemText(hListView, lvItem.iItem, 1, driveName);
            ListView_SetItemText(hListView, lvItem.iItem, 1, (LPWSTR)driveTypeStr);
            ListView_SetItemText(hListView, lvItem.iItem, 2, fileSystemName);

            WCHAR sizeStr[64];
            swprintf_s(sizeStr, 64, L"%llu GB", totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024));
            ListView_SetItemText(hListView, lvItem.iItem, 3, sizeStr);

            swprintf_s(sizeStr, 64, L"%llu GB", usedBytes / (1024 * 1024 * 1024));
            ListView_SetItemText(hListView, lvItem.iItem, 4, sizeStr);

            // Codepage and physical address are not directly available, so we leave them empty for now
            ListView_SetItemText(hListView, lvItem.iItem, 5, (LPWSTR)L"");
            ListView_SetItemText(hListView, lvItem.iItem, 6, (LPWSTR)L"");
        }
    }
}


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
   
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
       CW_USEDEFAULT, 0, 600, 450, nullptr, nullptr, hInstance, nullptr);

   // Create the status bar
   hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"Ready", hWnd, IDC_STATUSBAR);

   if (!hWnd)
   {
      return FALSE;
   }

   // Set the parts of the status bar
   int statusWidths[] = { 450, -1 };
   SendMessage(hStatus, SB_SETPARTS, _countof(statusWidths), (LPARAM)statusWidths);
   SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Ready");

   // Create the ListView control for displaying the drives and volumes
   hListView = CreateWindowW(WC_LISTVIEW, L"",
       WS_CHILD | WS_VISIBLE | LVS_REPORT,
       0, 0, 590, 150, hWnd, nullptr, hInstance, nullptr);

   // Create the image list
   hImageList = ImageList_Create(16, 16, ILC_COLOR32, 1, 1);
   ListView_SetImageList(hListView, hImageList, LVSIL_SMALL);

   // Initialize the ListView columns
   LVCOLUMN lvColumn;
   lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   lvColumn.cx = 100;

   // Define the column text as non-const wchar_t arrays
   wchar_t iconText[] = L"Drive";
   wchar_t typeText[] = L"Type";
   wchar_t fileSystemText[] = L"File System";
   wchar_t totalSizeText[] = L"Total Size";
   wchar_t usedSizeText[] = L"Used Size";
   wchar_t codepageText[] = L"Codepage";
   wchar_t physicalAddressText[] = L"Physical Address";

   lvColumn.pszText = iconText;
   ListView_InsertColumn(hListView, 0, &lvColumn);

   lvColumn.pszText = typeText;
   ListView_InsertColumn(hListView, 1, &lvColumn);

   lvColumn.pszText = fileSystemText;
   ListView_InsertColumn(hListView, 2, &lvColumn);

   lvColumn.pszText = totalSizeText;
   ListView_InsertColumn(hListView, 3, &lvColumn);

   lvColumn.pszText = usedSizeText;
   ListView_InsertColumn(hListView, 4, &lvColumn);

   lvColumn.pszText = codepageText;
   ListView_InsertColumn(hListView, 5, &lvColumn);

   lvColumn.pszText = physicalAddressText;
   ListView_InsertColumn(hListView, 6, &lvColumn);

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

   // Populate the Drives ListView with initial drive information
   PopulateDrives();

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
            swprintf_s(dateTime, 100, L"%02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

            // Update the status bar
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Ready");
            SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)dateTime);
        }
        break;

    case WM_SIZE:
        // Resize the status bar
        SendMessage(hStatus, WM_SIZE, 0, 0);
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
        pMinMaxInfo->ptMinTrackSize.x = 600;
        pMinMaxInfo->ptMinTrackSize.y = 450;
        pMinMaxInfo->ptMaxTrackSize.x = 600;
        pMinMaxInfo->ptMaxTrackSize.y = 450;
        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_HELP:
        {
            // Check for the presence of the "Help.md" file
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(L"C:\\ProgramData\\LFSD_Manager\\Help\\Help.html", &findFileData);

            if (hFind != INVALID_HANDLE_VALUE)
            {
                // File exists, open it in MSEDGE.exe
				UpdateStatusBarText(0, L"Opening help file...");
                FindClose(hFind);
                ShellExecute(NULL, L"open", L"msedge.exe", L"C:\\ProgramData\\LFSD_Manager\\Help\\Help.html", NULL, SW_SHOWNORMAL);
            }
            else
            {
                // File does not exist, show the help dialog
				UpdateStatusBarText(0, L"Help file not found...");
                DialogBox(hInst, MAKEINTRESOURCE(IDD_HELPBOX), hWnd, Help);
            }
            break;
        }
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
            UpdateStatusBarText(0, L"Exiting...");
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