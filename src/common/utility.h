/**
 *  @file   utility.h
 *  @author ichenq@gmail.com
 *  @date   Oct 19, 2011
 *  @brief  common utilities
 */

#pragma once


#include "cmndef.h"
#include <assert.h>
#include <vector>


struct global_init
{
public:
    global_init();
    ~global_init();

private:
    global_init(const global_init&);
    global_init& operator = (const global_init&);
};


template <typename T>
_tstring    ToString(const T& obj)
{
    _tstringstream strm;
    strm << obj;
    return strm.str();
}

// get curent time
_tstring Now();

// last error message
_tstring GetErrorMessage(DWORD dwErrorCode);


// converts a sockaddr_in structure into a human-readable string
_tstring	AddressToString(const sockaddr_in& addr);


// converts a numeric string to a sockaddr_in structure
bool        StringToAddress(const _tstring& strAddr, sockaddr_in* pAddr);



// format mac address
std::string FormateMAC(const BYTE* pMac, size_t len);


// get mac address and push back to a vector
void    GetMAC(std::vector<std::string>& vec);

// 
inline HANDLE   CreateCompletionPort(size_t concurrency)
{
    return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrency);
}


inline bool     AssociateDevice(HANDLE hCompletionPort, HANDLE hDevice, ULONG_PTR completionkey)
{
    assert(hCompletionPort != INVALID_HANDLE_VALUE);
    return (::CreateIoCompletionPort(hDevice, hCompletionPort, completionkey, 0) == hCompletionPort);
}


#define LAST_ERROR_MSG      GetErrorMessage(::GetLastError()).c_str()


// send message to a thread
bool send_message_to(unsigned thread_id, 
                     unsigned msg, 
                     unsigned param1 /* = 0 */, 
                     long param2 /* = 0 */);
