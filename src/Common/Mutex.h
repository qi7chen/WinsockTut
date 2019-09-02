#pragma once

#include <Windows.h>

class Mutex
{
public:
	Mutex()
	{
		InitializeCriticalSection(&cs_);
	}

	~Mutex()
	{
		DeleteCriticalSection(&cs_);
	}

	void Lock()
	{
		EnterCriticalSection(&cs_);
	}

	bool TryLock()
	{
		return TryEnterCriticalSection(&cs_) > 0;
	}

	void UnLock()
	{
		LeaveCriticalSection(&cs_);
	}

private:
	CRITICAL_SECTION cs_;
};
