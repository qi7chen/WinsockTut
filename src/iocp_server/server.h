/**
 * Copyright (C) 2012-2014 ichenq@gmail.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */
 
#pragma once

#include "common/utility.h"


/* initialize iocp server internal data structures */
int     server_init(const char* host, short port);
void    server_destroy();

/* start run server */
int     server_run();
