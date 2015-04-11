/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "utility.h"
#include <time.h>
#include <stdlib.h>

#define THREAD_LOCAL    __declspec(thread)

const char*  Now()
{
    static THREAD_LOCAL char buffer[MAX_PATH];
    struct tm st;
    time_t now = time(NULL);
    localtime_s(&st, &now);
    strftime(buffer, _countof(buffer), ("%Y-%m-%d %H:%M:%S"), &st);
    return buffer;
}

const char* GetErrorMessage(DWORD dwErrorCode)
{
    static THREAD_LOCAL char description[MAX_PATH];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, description, MAX_PATH-1, NULL);
    return description;
}
