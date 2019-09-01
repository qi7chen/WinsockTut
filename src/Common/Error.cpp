// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Error.h"

#define THREAD_LOCAL    __declspec(thread)

const char* GetErrorMessage(DWORD dwErrorCode)
{
    static THREAD_LOCAL char description[MAX_PATH];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
        0, description, MAX_PATH-1, NULL);
    return description;
}
