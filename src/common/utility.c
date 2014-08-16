#include "utility.h"
#include <time.h>
#include <stdlib.h>

const char*  Now()
{
    static TLS char buffer[MAX_PATH];
    struct tm st;
    time_t now = time(NULL);
    localtime_s(&st, &now);
    strftime(buffer, _countof(buffer), ("%Y-%m-%d %H:%M:%S"), &st);
    return buffer;
}

const char* GetErrorMessage(DWORD dwErrorCode)
{
    static TLS char description[MAX_PATH];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, description, MAX_PATH-1, NULL);
    return description;
}
