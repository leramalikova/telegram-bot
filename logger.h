#pragma once

#include <fstream>
#include <unordered_map>

class Logger {
public:
    explicit Logger(const std::string& file_name);
    void AddSendLog(const std::string& what_where,
                    const std::unordered_map<std::string, std::string>& params);
    void AddGetLog(const std::string& what);
    void AddRecieveUpdateLog(const std::string& what);
    ~Logger();

private:
    std::ofstream logfile_;
    std::string MakeWhat(const std::unordered_map<std::string, std::string>& params);
};
