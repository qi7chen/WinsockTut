#pragma once


//
//  use the /FI option to force include this file
//

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT     0x0502      // Windows 2003
#endif


#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#if _MSC_VER < 1600
#error "This project need vc++ 10.0 or later to meet the new c++11 syntax"
#endif


// CRT
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include <tchar.h>
#include <process.h>

// SDK
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

// STL
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <functional>
#include <iterator>


