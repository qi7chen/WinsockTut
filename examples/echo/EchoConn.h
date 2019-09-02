// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include "PollerBase.h"
#include "PollEvent.h"

class EchoServer;

class EchoConn : public IPollEvent
{
public:
	explicit EchoConn(EchoServer* server, SOCKET fd);
	~EchoConn();

	void OnReadable();
	void OnWritable();
	void OnTimeout() {}

	void StartRead();
	void Close();

private:
	EchoServer* server_;
	SOCKET  fd_;
	int     cap_;
	int     size_;
	char*   buf_;
};
