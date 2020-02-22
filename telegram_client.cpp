#include <vector>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include "telegram_client.h"
#include "message_and_update.h"

class Message::MessageImpl {
public:
    explicit MessageImpl(Poco::SharedPtr<Poco::JSON::Object> value) : value_(value) {
    }

    int64_t GetChatId() const {
        auto chat = value_->getObject("chat");
        return chat->getValue<int>("id");
    }

    int GetId() const {
        return value_->getValue<int>("message_id");
    }

    std::string GetText() const {
        return value_->getValue<std::string>("text");
    }

    std::vector<std::string> GetBotCommands() const {
        std::vector<std::string> commands;
        if (value_->has("entities")) {
            auto entities = *(value_->getArray("entities"));
            for (auto& element : entities) {
                auto el = element.extract<Poco::JSON::Object::Ptr>();
                if (el->getValue<std::string>("type") == "bot_command") {
                    std::string text = GetText();
                    commands.push_back(
                        text.substr(el->getValue<int>("offset"), el->getValue<int>("length")));
                }
            }
        }
        return commands;
    }

private:
    Poco::SharedPtr<Poco::JSON::Object> value_;
};

Message CreateMessage(Poco::SharedPtr<Poco::JSON::Object> msg) {
    return Message(std::make_shared<Message::MessageImpl>(msg));
}

Message::Message(std::shared_ptr<MessageImpl> pimpl) : pimpl_(pimpl) {
}

int64_t Message::GetChatId() const {
    return pimpl_->GetChatId();
}

std::vector<std::string> Message::GetBotCommands() const {
    return pimpl_->GetBotCommands();
}

int Message::GetId() const {
    return pimpl_->GetId();
}

class Update::UpdateImpl {
public:
    explicit UpdateImpl(Poco::SharedPtr<Poco::JSON::Object> value) : value_(value) {
        std::vector<std::string> message_types = {"message", "edited_message", "channel_post",
                                                  "edited_channel_post"};
        for (auto& message_type : message_types) {
            if (value_->has(message_type)) {
                message_ = CreateMessage(value_->getObject(message_type));
                return;
            }
        }
    }

    bool IsMessage() const {
        return message_.has_value();
    }

    int GetId() const {
        return value_->getValue<int>("update_id");
    }

    Message GetMessage() const {
        return message_.value();
    }

private:
    std::optional<Message> message_;
    Poco::SharedPtr<Poco::JSON::Object> value_;
};

Update::Update(std::shared_ptr<UpdateImpl> pimpl) : pimpl_(pimpl) {
}

bool Update::IsMessage() const {
    return pimpl_->IsMessage();
}

int Update::GetId() const {
    return pimpl_->GetId();
}

Message Update::GetMessage() const {
    return pimpl_->GetMessage();
}

class TelegramClient::RequestHandler {
public:
    RequestHandler(const std::string& url, const std::string& token) : token_(token), url_(url) {
        Poco::URI uri(url);
        std::string scheme(uri.getScheme());
        std::string host(uri.getHost());
        auto port(uri.getPort());

        session = std::make_unique<Poco::Net::HTTPClientSession>(host, port);
    }

    Poco::Dynamic::Var SendRecieveGet(const std::string& request,
                                      const std::unordered_map<std::string, std::string>& params) {
        Poco::URI uri("/bot" + token_ + "/" + request);
        Poco::Net::HTTPRequest req;
        req.setMethod("GET");
        req.setContentType("application/json");
        Poco::Net::HTTPResponse response;

        for (const auto& param : params) {
            uri.addQueryParameter(param.first, param.second);
        }
        req.setURI(uri.getPathAndQuery());

        session->sendRequest(req);

        auto& body = session->receiveResponse(response);

        if (response.getStatus() != 200) {
            throw std::runtime_error(std::to_string(int(response.getStatus())) + " " +
                                     response.getReasonForStatus(response.getStatus()));
        }

        Poco::JSON::Parser parser;
        return parser.parse(body);
    }

    void SendPost(const std::string& request,
                  const std::unordered_map<std::string, std::string>& params) {
        Poco::URI uri("/bot" + token_ + "/" + request);
        Poco::Net::HTTPRequest req("POST", uri.toString());
        req.setContentType("application/json");
        Poco::Net::HTTPResponse response;

        Poco::JSON::Object obj;
        for (auto& elem : params) {
            obj.set(elem.first, elem.second);
        }
        std::stringstream ss;
        obj.stringify(ss);
        req.setContentLength(ss.str().length());

        obj.stringify(session->sendRequest(req));
        auto& body = session->receiveResponse(response);
        if (response.getStatus() != 200) {
            throw std::runtime_error(std::to_string(int(response.getStatus())) + " " +
                                     response.getReasonForStatus(response.getStatus()));
        }
    }

private:
    std::string token_;
    std::string url_;
    std::unique_ptr<Poco::Net::HTTPClientSession> session;
};

TelegramClient::TelegramClient(const std::string& token, const std::string& url)
    : logger_(std::make_unique<Logger>("/Users/valeriamalikova/Desktop/logs.txt")),
      request_handler_(std::make_shared<RequestHandler>(url, token)) {
}

std::vector<Update> TelegramClient::GetUpdates(int timeout, int offset) {
    std::unordered_map<std::string, std::string> params;
    if (timeout > 0) {
        params["timeout"] = std::to_string(timeout);
    }
    if (offset > 0) {
        params["offset"] = std::to_string(offset);
    }

    auto var_poco_updates = request_handler_->SendRecieveGet("getUpdates", params);
    logger_->AddGetLog("updates");
    auto poco_updates = var_poco_updates.extract<Poco::JSON::Object::Ptr>()->getArray("result");

    std::vector<Update> updates;
    for (auto& elem : *poco_updates) {
        logger_->AddRecieveUpdateLog(elem.toString());
        auto update_impl =
            std::make_shared<Update::UpdateImpl>(elem.extract<Poco::JSON::Object::Ptr>());
        updates.emplace_back(update_impl);
    }
    return updates;
}

void TelegramClient::GetMe() {
    std::unordered_map<std::string, std::string> params;
    logger_->AddGetLog("me");
    request_handler_->SendRecieveGet("getMe", params);
}

void TelegramClient::SendMessage(int64_t where_to, const std::string& text, int64_t as_reply_to) {
    std::unordered_map<std::string, std::string> params;
    params["text"] = text;
    params["chat_id"] = std::to_string(where_to);
    if (as_reply_to > 0) {
        params["reply_to_message_id"] = std::to_string(as_reply_to);
    }
    request_handler_->SendPost("sendMessage", params);
    logger_->AddSendLog("message", params);
}

void TelegramClient::SendGif(int64_t where, const std::string& text) {
    std::unordered_map<std::string, std::string> params;
    params["chat_id"] = std::to_string(where);
    params["animation"] = text;
    request_handler_->SendPost("sendAnimation", params);
    logger_->AddSendLog("gif", params);
}

void TelegramClient::SendPhoto(int64_t where, const std::string& text) {
    std::unordered_map<std::string, std::string> params;
    params["chat_id"] = std::to_string(where);
    params["photo"] = text;
    request_handler_->SendPost("sendPhoto", params);
    logger_->AddSendLog("photo", params);
}

void TelegramClient::SendSticker(int64_t where, const std::string& text) {
    std::unordered_map<std::string, std::string> params;
    params["chat_id"] = std::to_string(where);
    params["document"] = text;
    request_handler_->SendPost("sendDocument", params);
    logger_->AddSendLog("sticker", params);
}
