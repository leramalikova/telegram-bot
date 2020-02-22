#pragma once

#include "telegram_client.h"
#include <vector>

class Bot {
public:
    Bot(const std::string& token, const std::string& url);
    void Run();

private:
    std::unique_ptr<TelegramClient> client_;
    std::vector<std::string> photo_;
    std::vector<std::string> sticker_;
    std::vector<std::string> gif_;

    std::vector<std::string> ReadVectorFromFile(const std::string& file);
    void SetOffcet(int offset);
    int GetOffset() const;
    void RespondToCommand(const Update& update);
};
