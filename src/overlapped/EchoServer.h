/**
 * Copyright (C) 2012-2015 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

/*
 * A simple echo server implemented by overlapped I/O
 */
int StartEchoServer(const char* host, const char* port);
