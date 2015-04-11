/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

#include <stdlib.h>
#include <Windows.h>

/* default I/O buffer size */
#define  DEFAULT_BUFFER_SIZE 8192

/* type of I/O operation */
enum OperType
{
    OperClose,          /* socket is closed, default state */
    OperConnect,        /* connecting to another peer */
    OperAccept,         /* accept new client connection */
    OperSend,           /* sending data */
    OperRecv,           /* receive data */
    OperDisconnect,     /* disconnect an socket */
};

#define DEFAULT_HOST    "0.0.0.0"
#define DEFAULT_PORT    "9527"

/* current date */
const char* Now();

/* description of specified error id */
const char*  GetErrorMessage(DWORD dwError);

/* error description of current thread */
#define LAST_ERROR_MSG   GetErrorMessage(GetLastError())

#define CHECK(expr)   if (!(expr)) {                    \
    MessageBoxA(NULL, #expr, LAST_ERROR_MSG, MB_OK);    \
    abort(); }
