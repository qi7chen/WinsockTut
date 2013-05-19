//  A simple echo server use asynchrounous select model
//  by ichenq@gmail.com 
//  Nov 24, 2011



#include "appdef.h"
#include <assert.h>



#pragma comment(lib, "ws2_32")


#pragma warning(disable: 4996)



HWND IntiInstance(HINSTANCE hInstance);

// window procedure
LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


// initialize winsock and other environment
static global_init      g_global_init;


// main entry
int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 3)
    {
        _tprintf(_T("Usage: $program $host $port"));
        return 1;
    }

    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
    HWND hWnd = IntiInstance(hInstance);

    CHECK(InitializeServer(hWnd, argv[1], argv[2]));

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_SOCKET)
        {
            int event = WSAGETSELECTEVENT(msg.lParam);
            int error = WSAGETSELECTERROR(msg.lParam);
            HandleNetEvents(hWnd, msg.wParam, event, error);  
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseServer();
}

HWND IntiInstance(HINSTANCE hInstance)
{
    const TCHAR* szTitle = _T("async-select");

    HWND hWnd = CreateWindow(_T("button"),szTitle, WS_POPUP,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
        NULL, NULL, hInstance, NULL);

    CHECK(hWnd != NULL);

    ShowWindow(hWnd, SW_HIDE);
    return hWnd;
}
