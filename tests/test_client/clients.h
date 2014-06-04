/**
 *  @file    clients.h
 *  @author: ichenq@gmail.com
 *  @date:   Oct 19, 2011
 *  @brief:  TCP clients implemented by I/O completion port
 *
 */

#pragma once


/* initialize internal loop data structure */
int  loop_init();
void loop_destroy();

/* run I/O event loop */
int  loop_run(int timeout);

/* create TCP connections */
int  create_connections(const char* host, short port, int num);
