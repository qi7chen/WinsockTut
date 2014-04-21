#include "utility.h"
#include <time.h>


using std::string;


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

bool PrintLog(const char* fmt, ...)
{
    char buffer[BUFSIZ];
    va_list ap;
    va_start(ap, fmt);
    int count = vsprintf_s(buffer, fmt, ap);
    va_end(ap);
    if (count > 0)
    {
        count = printf("%s: %s", Now().data(), buffer);
        return count > 0;
    }
    return true;
}
