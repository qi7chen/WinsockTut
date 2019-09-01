// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <stdarg.h>
#include <string>

// Like printf, return a C++ string
std::string StringPrintf(const char* format, ...);

// Store result into a supplied string and return it
const std::string& SStringPrintf(std::string* dst, const char* format, ...);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
void StringAppendV(std::string* dst, const char* format, va_list ap);
