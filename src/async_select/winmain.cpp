#include "appdef.h"
#include <assert.h>
#include <Shellapi.h>



#pragma comment(lib, "Shell32")
#pragma warning(disable: 4996)


// Create a hiden window
HWND IntiInstance(HINSTANCE hInstance)
{
    const char* szTitle = "async-select";
    HWND hWnd = CreateWindowA("button", szTitle, WS_POPUP,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
        NULL, NULL, hInstance, NULL);

    CHECK(hWnd != NULL);

    ShowWindow(hWnd, SW_HIDE);
    return hWnd;
}

// main entry
int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: AsynSelect [host] [port]"));
        return 1;
    }

    WinsockInit init;
    HWND hWnd = IntiInstance((HINSTANCE)GetModuleHandle(NULL));
    CHECK(InitializeServer(hWnd, argv[1], atoi(argv[2])));
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {        
        if (msg.message == WM_SOCKET)
        {
            SOCKET fd = msg.wParam;
            int event = WSAGETSELECTEVENT(msg.lParam);
            int error = WSAGETSELECTERROR(msg.lParam);
            HandleNetEvents(hWnd, fd, event, error);  
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseServer();
}


