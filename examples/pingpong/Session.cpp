// Copyright (C) 2012-present prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#include "Session.h"
#include "Server.h"

Session::Session(Server* server, SOCKET fd)
    : server_(server), service_(server->GetIOService()), fd_(fd)
{
}

Session::~Session()
{
}

void Session::StartRead()
{

}

void Session::OnRead(int error, int bytes)
{

}

void Session::OnWritten(int error, int bytes)
{

}
