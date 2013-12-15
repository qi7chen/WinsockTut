/**
 *  @file   winmain.cpp
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  使用WSAAsyncSelect()实现的简单Echo Server
 *			
 */


#include "appdef.h"
#include <assert.h>
#include <Shellapi.h>



#pragma comment(lib, "Shell32")
#pragma warning(disable: 4996)


// 创建一个隐藏窗口
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

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ("Usage: $program $host $port"));
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


