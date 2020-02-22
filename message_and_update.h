#pragma once

#include <memory>
#include <string>
#include <vector>

class Message;

class Update {
public:
    class UpdateImpl;

    explicit Update(std::shared_ptr<UpdateImpl> pimpl);
    bool IsMessage() const;
    int GetId() const;
    Message GetMessage() const;

private:
    friend Message;

    std::shared_ptr<UpdateImpl> pimpl_;
};

class Message {
public:
    class MessageImpl;
    explicit Message(std::shared_ptr<MessageImpl> pimpl);

    int64_t GetChatId() const;
    int GetId() const;
    std::vector<std::string> GetBotCommands() const;

private:
    friend Update;
    friend Update::UpdateImpl;

    std::shared_ptr<MessageImpl> pimpl_;
};