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
#include <string>
#include <winioctl.h>
#include <initguid.h>
#include <KnownFolders.h>
#include <vector>
#include <algorithm>

// Define the missing GUIDs if they are not defined in the included headers
#ifndef PARTITION_SYSTEM_GUID
DEFINE_GUID(PARTITION_SYSTEM_GUID, 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7);
#endif

#ifndef PARTITION_BASIC_DATA_GUID
DEFINE_GUID(PARTITION_BASIC_DATA_GUID, 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7);
#endif

#ifndef PARTITION_MSFT_RESERVED_GUID
DEFINE_GUID(PARTITION_MSFT_RESERVED_GUID, 0xE3C9E316, 0x0B5C, 0x4DB8, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE);
#endif

#ifndef PARTITION_EFI_SYSTEM_GUID
DEFINE_GUID(PARTITION_EFI_SYSTEM_GUID, 0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B);
#endif


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
INT_PTR CALLBACK    UnknownDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Contribute(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Help(HWND, UINT, WPARAM, LPARAM);
HWND hStatus;
HWND hListViewDrives;
HWND hListViewVolumes;
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

void EnableDoubleBuffering(HWND hWnd) {
    // Enable double buffering for the ListView control
    ListView_SetExtendedListViewStyle(hWnd, LVS_EX_DOUBLEBUFFER);
}

// Structure to hold drive information
struct DriveInfo {
    std::wstring driveName;
    std::wstring driveType;
    std::wstring fileSystemName;
    ULONGLONG totalSize;
    ULONGLONG usedSize;
};

// Structure to hold volume information
struct VolumeInfo {
    std::wstring volumePath;
    std::wstring driveType;
    std::wstring fileSystemName;
    ULONGLONG totalSize;
    ULONGLONG usedSize;
    std::wstring partitionType;
};

// Global variables to store the previous state
std::vector<DriveInfo> previousDrives;
std::vector<VolumeInfo> previousVolumes;

bool CompareDrives(const std::vector<DriveInfo>& newDrives) {
    if (newDrives.size() != previousDrives.size()) {
        return false;
    }
    for (size_t i = 0; i < newDrives.size(); ++i) {
        if (newDrives[i].driveName != previousDrives[i].driveName ||
            newDrives[i].driveType != previousDrives[i].driveType ||
            newDrives[i].fileSystemName != previousDrives[i].fileSystemName ||
            newDrives[i].totalSize != previousDrives[i].totalSize ||
            newDrives[i].usedSize != previousDrives[i].usedSize) {
            return false;
        }
    }
    return true;
}

bool CompareVolumes(const std::vector<VolumeInfo>& newVolumes) {
    if (newVolumes.size() != previousVolumes.size()) {
        return false;
    }
    for (size_t i = 0; i < newVolumes.size(); ++i) {
        if (newVolumes[i].volumePath != previousVolumes[i].volumePath ||
            newVolumes[i].driveType != previousVolumes[i].driveType ||
            newVolumes[i].fileSystemName != previousVolumes[i].fileSystemName ||
            newVolumes[i].totalSize != previousVolumes[i].totalSize ||
            newVolumes[i].usedSize != previousVolumes[i].usedSize ||
            newVolumes[i].partitionType != previousVolumes[i].partitionType) {
            return false;
        }
    }
    return true;
}

void PopulateDrives() {
    UpdateStatusBarText(0, L"Updating drives...");

    // Save the current selection
    int selectedIndex = ListView_GetNextItem(hListViewDrives, -1, LVNI_SELECTED);
    std::wstring selectedDrive;
    if (selectedIndex != -1) {
        WCHAR buffer[MAX_PATH];
        ListView_GetItemText(hListViewDrives, selectedIndex, 0, buffer, MAX_PATH);
        selectedDrive = buffer;
    }

    std::vector<DriveInfo> newDrives;

    // Add the drives and partitions to the ListView
    DWORD drives = GetLogicalDrives();
    WCHAR driveName[] = L"A:\\";
    for (int i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            driveName[0] = L'A' + i;

            // Get drive type
            UINT driveType = GetDriveType(driveName);
            const wchar_t* driveTypeStr = L"Unknown";
            switch (driveType) {
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

            // Store the drive information
            DriveInfo driveInfo = { driveName, driveTypeStr, fileSystemName, totalNumberOfBytes.QuadPart, usedBytes };
            newDrives.push_back(driveInfo);
        }
    }

    // Compare with the previous state and update if there are changes
    if (!CompareDrives(newDrives)) {
        previousDrives = newDrives;

        // Clear the existing items
        ListView_DeleteAllItems(hListViewDrives);
        ImageList_RemoveAll(hImageList);

        for (const auto& drive : newDrives) {
            // Get the drive icon
            SHFILEINFO shFileInfo;
            SHGetFileInfo(drive.driveName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
            int iconIndex = ImageList_AddIcon(hImageList, shFileInfo.hIcon);
            DestroyIcon(shFileInfo.hIcon);

            // Insert the drive information into the ListView
            LVITEM lvItem = { 0 };
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iItem = ListView_GetItemCount(hListViewDrives);
            lvItem.iSubItem = 0;
            lvItem.pszText = const_cast<LPWSTR>(drive.driveName.c_str());
            lvItem.iImage = iconIndex;
            ListView_InsertItem(hListViewDrives, &lvItem);

            ListView_SetItemText(hListViewDrives, lvItem.iItem, 1, const_cast<LPWSTR>(drive.driveType.c_str()));
            ListView_SetItemText(hListViewDrives, lvItem.iItem, 2, const_cast<LPWSTR>(drive.fileSystemName.c_str()));

            WCHAR sizeStr[64];
            swprintf_s(sizeStr, 64, L"%llu GB", drive.totalSize / (1024 * 1024 * 1024));
            ListView_SetItemText(hListViewDrives, lvItem.iItem, 3, sizeStr);

            swprintf_s(sizeStr, 64, L"%llu GB", drive.usedSize / (1024 * 1024 * 1024));
            ListView_SetItemText(hListViewDrives, lvItem.iItem, 4, sizeStr);

            // Physical address not directly available, so we leave empty for now
            ListView_SetItemText(hListViewDrives, lvItem.iItem, 5, (LPWSTR)L"");
        }

        // Restore the selection
        if (!selectedDrive.empty()) {
            LVFINDINFO findInfo = { 0 };
            findInfo.flags = LVFI_STRING;
            findInfo.psz = selectedDrive.c_str();
            int newIndex = ListView_FindItem(hListViewDrives, -1, &findInfo);
            if (newIndex != -1) {
                ListView_SetItemState(hListViewDrives, newIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(hListViewDrives, newIndex, FALSE);
            }
        }
    }
}

void PopulateVolumes() {
    UpdateStatusBarText(0, L"Updating volumes...");

    // Save the current selection
    int selectedIndex = ListView_GetNextItem(hListViewVolumes, -1, LVNI_SELECTED);
    std::wstring selectedVolume;
    if (selectedIndex != -1) {
        WCHAR buffer[MAX_PATH];
        ListView_GetItemText(hListViewVolumes, selectedIndex, 0, buffer, MAX_PATH);
        selectedVolume = buffer;
    }

    std::vector<VolumeInfo> newVolumes;

    // Add the volumes to the ListView
    WCHAR volumeName[MAX_PATH];
    HANDLE hFindVolume = FindFirstVolume(volumeName, ARRAYSIZE(volumeName));
    if (hFindVolume == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        // Get the friendly name of the volume
        WCHAR volumePath[MAX_PATH] = { 0 };
        DWORD returnLength = 0;
        if (!GetVolumePathNamesForVolumeName(volumeName, volumePath, ARRAYSIZE(volumePath), &returnLength)) {
            wcscpy_s(volumePath, L"Unknown");
        }

        // Get volume type
        UINT driveType = GetDriveType(volumePath);
        const wchar_t* driveTypeStr = L"Unknown";
        switch (driveType) {
        case DRIVE_REMOVABLE: driveTypeStr = L"Removable"; break;
        case DRIVE_FIXED: driveTypeStr = L"Fixed"; break;
        case DRIVE_REMOTE: driveTypeStr = L"Network"; break;
        case DRIVE_CDROM: driveTypeStr = L"CD-ROM"; break;
        case DRIVE_RAMDISK: driveTypeStr = L"RAM Disk"; break;
        }

        // Get file system type
        WCHAR fileSystemName[MAX_PATH] = { 0 };
        GetVolumeInformation(volumePath, NULL, 0, NULL, NULL, NULL, fileSystemName, MAX_PATH);

        // Get total and used size
        ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
        GetDiskFreeSpaceEx(volumePath, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes);
        ULONGLONG usedBytes = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;

        // Get partition type
        WCHAR partitionTypeStr[64] = L"Unknown";
        HANDLE hVolume = CreateFile(volumePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hVolume != INVALID_HANDLE_VALUE) {
            PARTITION_INFORMATION_EX partitionInfo;
            DWORD bytesReturned;
            if (DeviceIoControl(hVolume, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, &partitionInfo, sizeof(partitionInfo), &bytesReturned, NULL)) {
                switch (partitionInfo.PartitionStyle) {
                case PARTITION_STYLE_MBR:
                    swprintf_s(partitionTypeStr, 64, L"MBR (Type: 0x%02X)", partitionInfo.Mbr.PartitionType);
                    break;
                case PARTITION_STYLE_GPT:
                    if (IsEqualGUID(partitionInfo.Gpt.PartitionType, PARTITION_SYSTEM_GUID)) {
                        swprintf_s(partitionTypeStr, 64, L"System");
                    }
                    else if (IsEqualGUID(partitionInfo.Gpt.PartitionType, PARTITION_MSFT_RESERVED_GUID)) {
                        swprintf_s(partitionTypeStr, 64, L"Reserved");
                    }
                    else if (IsEqualGUID(partitionInfo.Gpt.PartitionType, PARTITION_BASIC_DATA_GUID)) {
                        swprintf_s(partitionTypeStr, 64, L"Basic Data");
                    }
                    else if (IsEqualGUID(partitionInfo.Gpt.PartitionType, PARTITION_EFI_SYSTEM_GUID)) {
                        swprintf_s(partitionTypeStr, 64, L"EFI");
                    }
                    else {
                        swprintf_s(partitionTypeStr, 64, L"GPT");
                    }
                    break;
                case PARTITION_STYLE_RAW:
                    swprintf_s(partitionTypeStr, 64, L"RAW");
                    break;
                }
            }
            CloseHandle(hVolume);
        }

        // Store the volume information
        VolumeInfo volumeInfo = { volumePath, driveTypeStr, fileSystemName, totalNumberOfBytes.QuadPart, usedBytes, partitionTypeStr };
        newVolumes.push_back(volumeInfo);

    } while (FindNextVolume(hFindVolume, volumeName, ARRAYSIZE(volumeName)));

    FindVolumeClose(hFindVolume);

    // Compare with the previous state and update if there are changes
    if (!CompareVolumes(newVolumes)) {
        previousVolumes = newVolumes;

        // Clear the existing items
        ListView_DeleteAllItems(hListViewVolumes);

        for (const auto& volume : newVolumes) {
            // Get the volume icon
            SHFILEINFO shFileInfo = { 0 };
            SHGetFileInfo(volume.volumePath.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
            int iconIndex = ImageList_AddIcon(hImageList, shFileInfo.hIcon);
            DestroyIcon(shFileInfo.hIcon);

            // Insert the volume information into the ListView
            LVITEM lvItem = { 0 };
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iItem = ListView_GetItemCount(hListViewVolumes);
            lvItem.iSubItem = 0;
            lvItem.pszText = const_cast<LPWSTR>(volume.volumePath.c_str());
            lvItem.iImage = iconIndex;
            ListView_InsertItem(hListViewVolumes, &lvItem);

            ListView_SetItemText(hListViewVolumes, lvItem.iItem, 1, const_cast<LPWSTR>(volume.driveType.c_str()));
            ListView_SetItemText(hListViewVolumes, lvItem.iItem, 2, const_cast<LPWSTR>(volume.fileSystemName.c_str()));

            WCHAR sizeStr[64];
            swprintf_s(sizeStr, 64, L"%llu GB", volume.totalSize / (1024 * 1024 * 1024));
            ListView_SetItemText(hListViewVolumes, lvItem.iItem, 3, sizeStr);

            swprintf_s(sizeStr, 64, L"%llu GB", volume.usedSize / (1024 * 1024 * 1024));
            ListView_SetItemText(hListViewVolumes, lvItem.iItem, 4, sizeStr);

            ListView_SetItemText(hListViewVolumes, lvItem.iItem, 5, const_cast<LPWSTR>(volume.partitionType.c_str()));
        }

        // Restore the selection
        if (!selectedVolume.empty()) {
            LVFINDINFO findInfo = { 0 };
            findInfo.flags = LVFI_STRING;
            findInfo.psz = selectedVolume.c_str();
            int newIndex = ListView_FindItem(hListViewVolumes, -1, &findInfo);
            if (newIndex != -1) {
                ListView_SetItemState(hListViewVolumes, newIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                ListView_EnsureVisible(hListViewVolumes, newIndex, FALSE);
            }
        }
    }
}


void ResizeDriveViewColumns()
{
    UpdateStatusBarText(0, L"Resizing columns...");
    RECT rect;
    GetClientRect(hListViewDrives, &rect);
    int listViewWidth = rect.right - rect.left;

    // Number of columns
    int numColumns = 6;

    // Calculate the width of each column
    int columnWidth = listViewWidth / numColumns;

    // Set the width of each column
    for (int i = 0; i < numColumns; ++i)
    {
        ListView_SetColumnWidth(hListViewDrives, i, columnWidth);
    }
}

void ResizeVolumeViewColumns()
{
    UpdateStatusBarText(0, L"Resizing columns...");
    RECT rect;
    GetClientRect(hListViewDrives, &rect);
    int listViewWidth = rect.right - rect.left;

    // Number of columns
    int numColumns = 7;

    // Calculate the width of each column
    int columnWidth = listViewWidth / numColumns;

    // Set the width of each column
    for (int i = 0; i < numColumns; ++i)
    {
        ListView_SetColumnWidth(hListViewVolumes, i, columnWidth);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, 0, 600, 450, nullptr, nullptr, hInstance, nullptr);

    // Create the status bar
    hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"Ready", hWnd, IDC_STATUSBAR);

    if (!hWnd) {
        return FALSE;
    }

    // Set the parts of the status bar
    int statusWidths[] = { 450, -1 };
    SendMessage(hStatus, SB_SETPARTS, _countof(statusWidths), (LPARAM)statusWidths);
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Ready");

    // Create the ListView controls for displaying the drives and volumes
    hListViewDrives = CreateWindowW(WC_LISTVIEW, L"Drives",
        WS_CHILD | WS_VISIBLE | WS_EX_CLIENTEDGE | LVS_REPORT,
        0, 0, 590, 150, hWnd, nullptr, hInstance, nullptr);
    hListViewVolumes = CreateWindowW(WC_LISTVIEW, L"Volumes",
        WS_CHILD | WS_VISIBLE | WS_EX_CLIENTEDGE | LVS_REPORT,
        0, 160, 590, 150, hWnd, nullptr, hInstance, nullptr);

    // Enable double buffering for the ListView controls
    EnableDoubleBuffering(hListViewDrives);
    EnableDoubleBuffering(hListViewVolumes);

    // Create the image list
    hImageList = ImageList_Create(16, 16, ILC_COLOR32, 1, 1);
    ListView_SetImageList(hListViewDrives, hImageList, LVSIL_SMALL);

    // Initialize the ListView columns
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvColumn.cx = 100;

    // Define the Drives column text as non-const wchar_t arrays
    wchar_t iconText[] = L"Drive";
    wchar_t typeText[] = L"Type";
    wchar_t fileSystemText[] = L"File System";
    wchar_t totalSizeText[] = L"Total Size";
    wchar_t usedSizeText[] = L"Used Size";
    wchar_t physicalAddressText[] = L"Physical";

    // Define the Volumes column text as non-const wchar_t arrays
    wchar_t iconTextv[] = L"";
    wchar_t typeTextv[] = L"Type";
    wchar_t fileSystemTextv[] = L"File System";
    wchar_t totalSizeTextv[] = L"Total Size";
    wchar_t usedSizeTextv[] = L"Used Size";
    wchar_t partitionTypeTextv[] = L"Partition";
    wchar_t physicalDiskTextv[] = L"Physical";

    lvColumn.pszText = iconText;
    ListView_InsertColumn(hListViewDrives, 0, &lvColumn);
    lvColumn.pszText = iconTextv;
    ListView_InsertColumn(hListViewVolumes, 0, &lvColumn);

    lvColumn.pszText = typeText;
    ListView_InsertColumn(hListViewDrives, 1, &lvColumn);
    lvColumn.pszText = typeTextv;
    ListView_InsertColumn(hListViewVolumes, 1, &lvColumn);

    lvColumn.pszText = fileSystemText;
    ListView_InsertColumn(hListViewDrives, 2, &lvColumn);
    lvColumn.pszText = fileSystemTextv;
    ListView_InsertColumn(hListViewVolumes, 2, &lvColumn);

    lvColumn.pszText = totalSizeText;
    ListView_InsertColumn(hListViewDrives, 3, &lvColumn);
    lvColumn.pszText = totalSizeTextv;
    ListView_InsertColumn(hListViewVolumes, 3, &lvColumn);

    lvColumn.pszText = usedSizeText;
    ListView_InsertColumn(hListViewDrives, 4, &lvColumn);
    lvColumn.pszText = usedSizeTextv;
    ListView_InsertColumn(hListViewVolumes, 4, &lvColumn);

    lvColumn.pszText = physicalAddressText;
    ListView_InsertColumn(hListViewDrives, 5, &lvColumn);

    lvColumn.pszText = partitionTypeTextv;
    ListView_InsertColumn(hListViewVolumes, 5, &lvColumn);

    lvColumn.pszText = physicalDiskTextv;
    ListView_InsertColumn(hListViewVolumes, 6, &lvColumn);

    // Resize the columns of the ListView to fit the window
    ResizeDriveViewColumns();
    ResizeVolumeViewColumns();

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
    PopulateVolumes();

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static UINT_PTR timerId;
    static UINT_PTR updateTimerId;

    switch (message) {
    case WM_CREATE:
        // Set a timer to update the status bar every second
        timerId = SetTimer(hWnd, 1, 1000, NULL);
        // Set a timer to check for new drives and volumes every 30 seconds
        updateTimerId = SetTimer(hWnd, 2, 30000, NULL);
        break;

    case WM_TIMER:
        if (wParam == 1) {
            // Get the current date and time
            SYSTEMTIME st;
            GetLocalTime(&st);
            wchar_t dateTime[100];
            swprintf_s(dateTime, 100, L"%02d/%02d/%04d %02d:%02d:%02d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

            // Update the status bar
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Ready");
            SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)dateTime);
        }
        else if (wParam == 2) {
            // Check for new drives and volumes every 30 seconds
            PopulateDrives();
            PopulateVolumes();
        }
        break;

    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->hwndFrom == ListView_GetHeader(hListViewDrives)) {
            if (pnmhdr->code == HDN_BEGINTRACKW || pnmhdr->code == HDN_BEGINTRACKA) {
                return TRUE; // Prevent column resizing
            }
            else if (pnmhdr->code == HDN_ITEMCHANGINGW || pnmhdr->code == HDN_ITEMCHANGINGA) {
                LPNMHEADER pnmHeader = (LPNMHEADER)lParam;
                if (!(pnmHeader->pitem->mask & HDI_WIDTH)) {
                    return TRUE; // Prevent column deletion or other modifications
                }
            }
        }
        break;
    }

    case WM_SIZE:
        // Resize the status bar
        SendMessage(hStatus, WM_SIZE, 0, 0);
        // Resize the drive view columns
        ResizeDriveViewColumns();
        break;

    case WM_GETMINMAXINFO: {
        MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
        pMinMaxInfo->ptMinTrackSize.x = 600;
        pMinMaxInfo->ptMinTrackSize.y = 450;
        pMinMaxInfo->ptMaxTrackSize.x = 600;
        pMinMaxInfo->ptMaxTrackSize.y = 450;
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_HELP: {
            // Check for the presence of the "Help.md" file
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(L"C:\\ProgramData\\LFSD_Manager\\Help\\Help.html", &findFileData);

            if (hFind != INVALID_HANDLE_VALUE) {
                // File exists, open it in MSEDGE.exe
                UpdateStatusBarText(0, L"Opening help file...");
                FindClose(hFind);
                ShellExecute(NULL, L"open", L"msedge.exe", L"C:\\ProgramData\\LFSD_Manager\\Help\\Help.html", NULL, SW_SHOWNORMAL);
            }
            else {
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
            DialogBox(hInst, MAKEINTRESOURCE(IDD_UNKNOWNBOX), hWnd, UnknownDlg);
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

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
                 break;

    case WM_ERASEBKGND:
        // Prevent background from being erased to reduce flickering
        return 1;

    case WM_DESTROY:
        KillTimer(hWnd, timerId);
        KillTimer(hWnd, updateTimerId);
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
INT_PTR CALLBACK UnknownDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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