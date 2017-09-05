/**
 * Copyright (C) 2012-2017 ichenq@outlook.com. All rights reserved.
 * Distributed under the terms and conditions of the Apache License. 
 * See accompanying files LICENSE.
 */


#include <time.h>
#include <map>
#include <string>
#include <vector>

// a single chat message
struct ChatMessage
{
    time_t      timestamp;
    std::string name;
    std::string content;
};

// a chat room
class ChatRoom
{
public:
    ChatRoom();
    ~ChatRoom();

    int EnterRoom(const std::string& name);
    void LeaveRoom(int uid);

    void AddChat(const std::string& name, const std::string& content);

    const std::vector<ChatMessage>& GetChatHistory() const
    {
        return messages_;
    }

private:
    int                         nextUserId;
    std::map<int, std::string>  users_;
    std::vector<ChatMessage>    messages_;
};