#include <iostream>
#include "bot.h"

int main(int argc, char* argv[]) {
    try {
        std::string token = "1032816597:AAElHP4tgg_Xu9hkEwmmen-EG6ppmxKBEtc";
        std::string url = "http://35.207.130.78";
        Bot bot(token, url);
        bot.Run();
        return 0;
    } catch (const std::exception& exception) {
        std::cout << "BAD CRASH WITH " << exception.what() << '\n';
    }
}