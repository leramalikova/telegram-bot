#include <Poco/URI.h>
#include <iostream>
#include "bot.h"

Bot::Bot(const std::string& token, const std::string& url)
    : client_(std::make_unique<TelegramClient>(token, url)) {
    sticker_ = ReadVectorFromFile("/Users/valeriamalikova/shad-cpp0/bot/telegram/sticker.txt");
    photo_ = ReadVectorFromFile("/Users/valeriamalikova/shad-cpp0/bot/telegram/photo.txt");
    gif_ = ReadVectorFromFile("/Users/valeriamalikova/shad-cpp0/bot/telegram/gif.txt");
}

void Bot::Run() {
    while (true) {
        try {
            auto updates = client_->GetUpdates(200, GetOffset());
            for (auto& upd : updates) {
                SetOffcet(upd.GetId() + 1);
                RespondToCommand(upd);
            }
        } catch (const std::exception& ex) {
            if (static_cast<std::string>(ex.what()) == "stop") {
                return;
            }
        }
    }
}

int Bot::GetOffset() const {
    std::ifstream file;
    //ios::base::app??
    file.open("/Users/valeriamalikova/shad-cpp0/bot/telegram/offset.txt");
    int offset;
    file >> offset;
    return offset;
}

void Bot::SetOffcet(int offset) {
    std::ofstream file;
    //ios::base::app??
    file.open("/Users/valeriamalikova/shad-cpp0/bot/telegram/offset.txt");
    file << offset;
}

void Bot::RespondToCommand(const Update& update) {
    if (update.IsMessage()) {
        auto message = update.GetMessage();
        auto message_id = message.GetId();
        auto commands = message.GetBotCommands();

        for (const auto& command : commands) {
            if (command == "/random") {
                client_->SendMessage(message.GetChatId(), std::to_string(std::rand()));
                return;
            }
            if (command == "/weather") {
                client_->SendMessage(message.GetChatId(), "Winter is coming", message_id);
                return;
            }
            if (command == "/styleguide") {
                std::string msg("You want a joke?! Maybe next time... ");
                client_->SendMessage(message.GetChatId(), msg);
                client_->SendSticker(message.GetChatId(), "CAADAgADSQADXDsoBAkwbcw5FEw-FgQ");
                return;
            }
            if (command == "/stop") {
                client_->SendMessage(message.GetChatId(), "NO ONE CAN STOP ME HAHAHA");
                return;
                //throw std::runtime_error("stop");
            }
            if (command == "/crash") {
                client_->SendMessage(message.GetChatId(), "ARE YOU TRYING TO KILL ME?!");
                return;
                //abort();
            }
            if (command == "/gif" && !gif_.empty()) {
                auto index = std::rand() % gif_.size();
                client_->SendGif(message.GetChatId(), gif_[index]);
                return;
            }
            if (command == "/photo" && !photo_.empty()) {
                auto index = std::rand() % photo_.size();
                client_->SendPhoto(message.GetChatId(), photo_[index]);
                return;
            }
            if (command == "/sticker" && !sticker_.empty()) {
                auto index = std::rand() % sticker_.size();
                client_->SendSticker(message.GetChatId(), sticker_[index]);
                return;
            }
            if (command == "/help" || command == "/start") {
                std::string help =
                    "/random random number\n"
                    "/sticker random sticker\n"
                    "/photo random photo of my cat\n"
                    "/gif random gif\n"
                    "/crash crash me\n"
                    "/stop stop me\n"
                    "/styleguide bad joke from me\n"
                    "/weather weather forecast from Winterfell";
                client_->SendMessage(message.GetChatId(), help);
                return;
            }
            client_->SendMessage(message.GetChatId(), "What? Maybe you need /help?");
        }
    }
}

std::vector<std::string> Bot::ReadVectorFromFile(const std::string& file_name) {
    std::ifstream file(file_name);
    std::vector<std::string> result;
    std::string temp;
    while (std::getline(file, temp)) {
        result.push_back(temp);
    }
    return result;
}