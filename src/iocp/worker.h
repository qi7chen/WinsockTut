//  I/O Completion Port worker thread
//  by ichenq@gmail.com at Oct 19, 2011

#pragma once


// timeout milliseconds
enum {MAX_TIMEOUT = 50};


// workder thread entry
unsigned __stdcall NativeThreadFunc(void* param);

