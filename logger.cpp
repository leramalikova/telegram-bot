#include "logger.h"

Logger::Logger(const std::string& file_name) {
    logfile_.open(file_name, std::ios::app);
    logfile_ << "\n"
                "___________________________________\n"
                "___________LOGGING BEGINS___________"
                "____________________________________\n";
}

Logger::~Logger() {
    logfile_ << "\n"
                "___________________________________\n"
                "___________LOGGING ENDS_____________"
                "____________________________________\n";
}

void Logger::AddSendLog(const std::string& what_where,
                        const std::unordered_map<std::string, std::string>& params) {
    logfile_ << "_______Sent " << what_where << MakeWhat(params) << '\n';
}

void Logger::AddGetLog(const std::string& what) {
    logfile_ << "_______Requested to GET " << what << '\n';
}

void Logger::AddRecieveUpdateLog(const std::string& what) {
    logfile_ << "_______Recieved " << what << '\n';
}

std::string Logger::MakeWhat(const std::unordered_map<std::string, std::string>& params) {
    std::string result;
    for (const auto& elem : params) {
        result += " { " + elem.first + " : " + elem.second + " } ";
    }
    return result;
}