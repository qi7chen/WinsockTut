// Copyright (C) 2012-present ichenq@outlook.com All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "EchoConn.h"
#include "SocketOpts.h"
#include "EchoServer.h"

enum
{
    MAX_CONN_RECVBUF = 1024,
    MAX_RECV_COUNT = 5,
};

EchoConn::EchoConn(EchoServer* server, SOCKET fd)
	: server_(server), fd_(fd), recv_count_(0)
{
    buf_.resize(MAX_CONN_RECVBUF);
}

EchoConn::~EchoConn()
{
}

void EchoConn::Close()
{
	if (fd_ != INVALID_SOCKET)
	{
		SOCKET fd = fd_;
		fd_ = INVALID_SOCKET;
		server_->CloseSession(fd); // delete this
	}
}

void EchoConn::StartRead()
{
	int nbytes = ReadSome(fd_, &buf_[0], (int)buf_.size());
	if (nbytes < 0)
	{
		Close(); // EOF or error;
		return;
	}
    else if (nbytes > 0)
    {
        recv_count_++;
        fprintf(stdout, "%d recv %d bytes, %d\n", (int)fd_, nbytes, recv_count_);
        if (recv_count_ < MAX_RECV_COUNT)
        {
            // echo back
            nbytes = WriteSome(fd_, &buf_[0], nbytes);
            if (nbytes < 0)
            {
                Close();
                return;
            }
            fprintf(stdout, "%d send %d bytes, %d\n", (int)fd_, nbytes, recv_count_);
        }
        else
        {
            Close();
        }
    }
}

void EchoConn::OnReadable()
{
	StartRead();
}

void EchoConn::OnWritable()
{
	// writable
}
