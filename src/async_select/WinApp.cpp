/**
 *  @brief:  A simple echo server, use winsock Asynchronous Select I/O model
 *
 * 	@author: ichenq@gmail.com
 *  @date:   Nov 24, 2011
 */

#include "resource.h"
#include "WinApp.h"
#include <Windows.h>
#include <tchar.h>

#pragma warning(disable: 4996)






INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);




int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ASYNCSELECT), NULL, DlgProc, 0);

}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
        }
        break;

    case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDCANCEL:
                {
                    EndDialog(hDlg, 0);
                }
                break;
            }
        }
        break;

    case WM_DROPFILES:
        {

        }
        break;

    }

    return 0;
}