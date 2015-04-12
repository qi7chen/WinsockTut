/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

/*
 * TCP clients implemented by I/O completion port
 */
int StartEchoClient(int count, const char* host, const char* port);