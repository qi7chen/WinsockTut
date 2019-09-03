// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "ChatSession.h"
#include "ChatServer.h"

ChatSession::ChatSession(ChatServer* server, SOCKET fd)
    : server_(server), service_(server->GetIOService()), fd_(fd)
{
}

ChatSession::~ChatSession()
{
}

void ChatSession::StartRead()
{

}

void ChatSession::OnRead(int error, int bytes)
{

}

void ChatSession::OnWritten(int error, int bytes)
{

}
