// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <Windows.h>

/// description of specified error id 
const char*  GetErrorMessage(DWORD dwError);

// last error description of current thread
#define LAST_ERROR_MSG   GetErrorMessage(GetLastError())
