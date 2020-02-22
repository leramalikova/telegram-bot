#pragma once

#include "message_and_update.h"
#include "logger.h"
#include <string>
#include <unordered_map>

class TelegramClient {
public:
    class RequestHandler;

    TelegramClient(const std::string& token, const std::string& url);
    std::vector<Update> GetUpdates(int timeout = 0, int offset = 0);
    void GetMe();
    void SendGif(int64_t where, const std::string& text);
    void SendPhoto(int64_t where, const std::string& text);
    void SendSticker(int64_t where, const std::string& text);
    void SendMessage(int64_t where, const std::string& text, int64_t as_reply_to = 0);

private:
    std::unique_ptr<Logger> logger_;
    std::shared_ptr<TelegramClient::RequestHandler> request_handler_;
};