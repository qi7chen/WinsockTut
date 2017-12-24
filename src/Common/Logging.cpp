// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>
#include "Mutex.h"
#include "StringPrintf.h"


static std::wstring Utf8ToWide(const std::string& strUtf8)
{
    std::wstring strWide;
    int count = MultiByteToWideChar(CP_UTF8, 0, strUtf8.data(), (int)strUtf8.length(), NULL, 0);
    if (count > 0)
    {
        strWide.resize(count);
        MultiByteToWideChar(CP_UTF8, 0, strUtf8.data(), (int)strUtf8.length(),
            const_cast<wchar_t*>(strWide.data()), (int)strWide.length());
    }
    return strWide;
}

const size_t MAX_OUTPUT_LEN = 4032;

static void OutputStringToDebugger(const std::string& message)
{
    const std::wstring& text = Utf8ToWide(message);
    if (text.size() < MAX_OUTPUT_LEN) //common case
    {
        OutputDebugStringW(text.c_str());
        return;
    }
    size_t outputed = 0;
    while (outputed < text.size())
    {
        // maximum length accepted
        // see http://www.unixwiz.net/techtips/outputdebugstring.html
        wchar_t buf[MAX_OUTPUT_LEN] = {};
        size_t left = text.size() - outputed;
        wcsncpy_s(buf, text.c_str() + outputed, min(left, MAX_OUTPUT_LEN-1));
        OutputDebugStringW(buf);
        if (left >= MAX_OUTPUT_LEN-1)
        {
            outputed += MAX_OUTPUT_LEN-1;
        }
        else
        {
            outputed += left;
        }
    }
}

namespace detail {

void DefaultLogHandler(LogLevel level, const char* filename, int line,
                       const std::string& message)
{
    static const char* level_names[] = { "DEBUG","INFO", "WARNING", "ERROR", "FATAL" };
    const char* sep = strrchr(filename, '\\');
    if (sep)
    {
        filename = sep + 1;
    }
    auto msg = StringPrintf("[%s %s:%d] %s\n", level_names[level], filename,
        line, message.c_str());
#ifdef _DEBUG
    fprintf(stderr, "%s\n", msg.c_str());
#else
    OutputStringToDebugger(msg);
#endif //_DEBUG
}

void NullLogHandler(LogLevel /* level */, const char* /* filename */,
                    int /* line */, const std::string& /* message */)
{
    // Nothing.
}

typedef void LogHandler(LogLevel level, const char* filename, int line,
    const std::string& message);

static LogHandler* log_handler_ = &DefaultLogHandler;
static int log_silencer_count_ = 0;
static Mutex log_silencer_count_mutex_;

void LogMessage::Finish() {
    bool suppress = false;

    if (level_ != LOGLEVEL_FATAL) {
        log_silencer_count_mutex_.Lock();
        suppress = log_silencer_count_ > 0;
        log_silencer_count_mutex_.UnLock();
    }

    if (!suppress) {
        log_handler_(level_, filename_, line_, strm_.str());
    }

    if (level_ == LOGLEVEL_FATAL) {
        abort();
    }
}

void LogFinisher::operator=(LogMessage& other) {
    other.Finish();
}

LogHandler* GetDefaultLogHandler()
{
    return log_handler_;
}

} // namespace detail
