/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

typedef unsigned __int16 uint16;

enum
{
    CMD_REG = 100,
    CMD_TALK = 101,
    CMD_LEAVE = 102,
};

struct Header
{
    uint16 size;
    uint16 command;
};