/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "Factory.h"
#include "select/SelectChatServer.h"

IChatServer* CreateChatServer(int mode)
{
    switch (mode)
    {
    case MODE_SELECT:
        return new SelectChatServer();
    }
    return NULL;
}