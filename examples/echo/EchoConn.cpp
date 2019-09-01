// Copyright (C) 2012-2018 . All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoConn.h"
#include "SocketOpts.h"
#include "EchoServer.h"

enum
{
	MAX_CONN_RECVBUF = 1024,
};

EchoConn::EchoConn(EchoServer* server, SOCKET fd)
	: poller_(server->Poller()), server_(server), fd_(fd)
{
	cap_ = MAX_CONN_RECVBUF;
	size_ = 0;
	buf_ = new char[cap_];
}

EchoConn::~EchoConn()
{
}

void EchoConn::Close()
{
	cap_ = 0;
	size_ = 0;
	if (buf_ != nullptr)
	{
		delete buf_;
		buf_ = nullptr;
	}

	if (fd_ != INVALID_SOCKET)
	{
		SOCKET fd = fd_;
		fd_ = INVALID_SOCKET;
		poller_->RemoveFd(fd);
		closesocket(fd);
		server_->Close(fd); // delete this
	}
}

void EchoConn::StartRead()
{
	int nbytes = ReadSome(fd_, buf_, cap_);
	if (nbytes <= 0)
	{
		Close(); // EOF or error;
		return;
	}
	size_ = nbytes;
	fprintf(stdout, "%d recv %d bytes\n", fd_, nbytes);

	// echo back
	nbytes = WriteSome(fd_, buf_, size_);
	if (nbytes < 0)
	{
		Close();
		return;
	}
	fprintf(stdout, "%d send %d bytes\n", fd_, nbytes);
}

void EchoConn::OnReadable()
{
	StartRead();
}

void EchoConn::OnWritable()
{
	// writable
}
