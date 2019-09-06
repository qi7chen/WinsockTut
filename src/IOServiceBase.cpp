// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "IOServiceBase.h"
#include "Common/Logging.h"
#include "Common/Error.h"
#include "OverlappedIOService.h"


IOServiceBase::IOServiceBase()
{
}

IOServiceBase::~IOServiceBase()
{
}


IOServiceBase* CreateIOService(IOServiceType type)
{
    switch(type)
    {
    case IOOverlapped:
        return new OverlappedIOService;

    default:
        LOG(ERROR) << "unsupported service type";
        return nullptr;
    }
}