#include "utility.h"
#include <time.h>
#include <process.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")

using std::string;

//////////////////////////////////////////////////////////////////////////
string  Now()
{    
    struct tm st = {};
    time_t now = time(NULL);
    localtime_s(&st, &now);
    char buffer[MAX_PATH];
    int count = strftime(buffer, _countof(buffer), ("%Y-%m-%d %H:%M:%S"), &st);
    return string(buffer, count);
}

string GetErrorMessage(DWORD dwErrorCode)
{
    char description[MAX_PATH];
    DWORD count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, description, MAX_PATH-1, NULL);
    return string(description, count);
}

DWORD StartThread(unsigned (CALLBACK* routine) (void*), int var)
{
    return _beginthreadex(NULL, 0, routine, (void*)var, 0, NULL);
}

WinsockInit::WinsockInit()
{
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms742213(v=vs.85).aspx
    WSADATA data = {};
    CHECK(WSAStartup(MAKEWORD(2, 2), &data) == 0);
}

WinsockInit::~WinsockInit()
{
    WSACleanup();
}


