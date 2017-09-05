/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */

#include "ChatRoom.h"

ChatRoom::ChatRoom()
{
    nextUserId = 1;
}

ChatRoom::~ChatRoom()
{
}


int ChatRoom::EnterRoom(const std::string& name)
{
    int uid = nextUserId++;
    users_[uid] = name;
    return uid;
}

void ChatRoom::LeaveRoom(int uid)
{
    users_.erase(uid);
}

void ChatRoom::AddChat(const std::string& name, const std::string& content)
{
    ChatMessage message;
    message.timestamp = time(NULL);
    message.name = name;
    message.content = content;
    messages_.push_back(message);
}