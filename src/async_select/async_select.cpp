#include "resource.h"
#include <tchar.h>
#include <Windows.h>


INT_PTR CALLBACK    DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);




int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ASYNC_SELECT), NULL, DlgProc, 0);
}


INT_PTR CALLBACK    DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch(uMsg)
    {
    case WM_INITDIALOG:
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;            
        }
        break;
    }

    return 0;
}