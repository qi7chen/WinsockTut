// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "IOServiceBase.h"

// I/O Completion Port
class CompletionPortService : public IOServiceBase
{
public:
    CompletionPortService();
    ~CompletionPortService();

private:
    HANDLE  completion_port_handle_;
};