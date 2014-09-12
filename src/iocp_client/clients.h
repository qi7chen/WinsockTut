/**
 * Copyright (C) 2014
 * Johnnie Chen, ichenq@gmail.com
 *  
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#pragma once

/*
 * TCP clients implemented by I/O completion port
 */

/* initialize internal loop data structure */
int  loop_init();
void loop_destroy();

/* run I/O event loop */
int  loop_run(int timeout);

/* create TCP connections */
int  create_connections(const char* host, short port, int num);
