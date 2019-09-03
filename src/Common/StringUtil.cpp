// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "StringUtil.h"
#include <stdarg.h>

#ifndef va_copy
// This is a hack, assuming va_list is simply a
// pointer into the stack and is safe to copy.
#define va_copy(dest, src) ((dest) = (src))
#endif

void StringAppendV(std::string* dst, const char* format, va_list ap)
{
    // First try with a small fixed size buffer
    static const int kSpaceLength = 1024;
    char space[kSpaceLength];
    // It's possible for methods that use a va_list to invalidate
    // the data in it upon use.  The fix is to make a copy
    // of the structure before using it and use that copy instead.
    va_list backup_ap;
    va_copy(backup_ap, ap);
    int result = vsprintf_s(space, kSpaceLength, format, backup_ap);
    va_end(backup_ap);

    if (result < kSpaceLength) 
    {
        if (result >= 0) // Normal case -- everything fit.
        {
            dst->append(space, result);
            return;
        }

        va_copy(backup_ap, ap);
        result = vsprintf_s(NULL, 0, format, backup_ap);
        va_end(backup_ap);

        if (result < 0) // Just an error.
        {
            return;
        }
    }

    // Increase the buffer size to the size requested by vsnprintf,
    // plus one for the closing \0.
    int length = result + 1;
    char* buf = new char[length];

    // Restore the va_list before we use it again
    va_copy(backup_ap, ap);
    result = vsprintf_s(buf, length, format, backup_ap);
    va_end(backup_ap);

    if (result >= 0 && result < length) // It fit
    {
        dst->append(buf, result);
    }
    delete[] buf;
}

std::string StringPrintf(const char* format, ...) 
{
    va_list ap;
    va_start(ap, format);
    std::string result;
    StringAppendV(&result, format, ap);
    va_end(ap);
    return result;
}

const std::string& SStringPrintf(std::string* dst, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    dst->clear();
    StringAppendV(dst, format, ap);
    va_end(ap);
    return *dst;
}
